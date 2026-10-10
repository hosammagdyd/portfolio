/* Hero background: PCB traces carrying signal pulses over an oscilloscope grid,
   with sweeping ventilator-style waveforms. Pure canvas, no assets. */
(function () {
    'use strict';

    var canvas = document.querySelector('.hero-canvas');
    var hero = canvas && canvas.closest('.hero');
    if (!canvas || !hero || !canvas.getContext) return;

    var ctx = canvas.getContext('2d');
    var root = document.documentElement;
    var reduceMotion = window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches;

    var CELL = 40;           // grid / trace snap size (CSS px)
    var WAVE_STEP = 3;       // horizontal resolution of the waveforms
    var SWEEP_SPEED = 150;   // waveform sweep, px per second
    var DIRS = [[1, 0], [1, 1], [0, 1], [-1, 1], [-1, 0], [-1, -1], [0, -1], [1, -1]];

    var w = 0, h = 0, dpr = 1;
    var accent = [45, 212, 191], second = [96, 165, 250], light = false;
    var traces = [], channels = [], cursor = 0, clock = 0;
    var mouse = { x: -1000, y: -1000, tx: -1000, ty: -1000 };
    var visible = true, rafId = 0, lastTime = 0;

    /* ---------- Colours (follow the theme toggle) ---------- */
    function parseColor(str) {
        str = str.trim();
        if (str[0] === '#') {
            var n = parseInt(str.length === 4 ? str.replace(/#(.)(.)(.)/, '$1$1$2$2$3$3') : str.slice(1), 16);
            return [(n >> 16) & 255, (n >> 8) & 255, n & 255];
        }
        var m = str.match(/\d+/g);
        return m ? [+m[0], +m[1], +m[2]] : [45, 212, 191];
    }
    function readTheme() {
        light = root.getAttribute('data-theme') === 'light';
        accent = parseColor(getComputedStyle(root).getPropertyValue('--accent'));
        second = light ? [37, 99, 235] : [96, 165, 250];
    }
    function rgba(c, a) { return 'rgba(' + c[0] + ',' + c[1] + ',' + c[2] + ',' + a + ')'; }

    /* ---------- Circuit traces ---------- */
    function rand(a, b) { return a + Math.random() * (b - a); }

    function makeTrace() {
        // Bias starting points to the right, away from the hero text.
        var x = Math.round(rand(w * 0.3, w) / CELL) * CELL;
        var y = Math.round(rand(0, h * 0.8) / CELL) * CELL;
        var dir = Math.floor(Math.random() * 8);
        var pts = [[x, y]];
        var segments = 3 + Math.floor(Math.random() * 5);
        for (var i = 0; i < segments; i++) {
            if (i > 0) dir = (dir + (Math.random() < 0.5 ? 1 : 7)) % 8; // PCB-style 45° turns
            var len = (1 + Math.floor(Math.random() * 4)) * CELL;
            x += DIRS[dir][0] * len;
            y += DIRS[dir][1] * len;
            pts.push([x, y]);
        }
        var lens = [0];
        for (var j = 1; j < pts.length; j++) {
            lens.push(lens[j - 1] + Math.hypot(pts[j][0] - pts[j - 1][0], pts[j][1] - pts[j - 1][1]));
        }
        var total = lens[lens.length - 1];
        return {
            pts: pts, lens: lens, total: total,
            pos: -rand(0, total * 2),       // negative = waiting to fire
            speed: rand(90, 190),
            tail: rand(50, 110),
            flash: 0,
            fade: 0.35 + 0.65 * Math.min(1, pts[0][0] / w)
        };
    }

    function pointAt(t, d) {
        if (d <= 0) return t.pts[0];
        if (d >= t.total) return t.pts[t.pts.length - 1];
        for (var i = 1; i < t.lens.length; i++) {
            if (d <= t.lens[i]) {
                var k = (d - t.lens[i - 1]) / (t.lens[i] - t.lens[i - 1]);
                var a = t.pts[i - 1], b = t.pts[i];
                return [a[0] + (b[0] - a[0]) * k, a[1] + (b[1] - a[1]) * k];
            }
        }
        return t.pts[t.pts.length - 1];
    }

    function strokeBetween(t, from, to) {
        from = Math.max(0, from); to = Math.min(t.total, to);
        if (to <= from) return;
        var p = pointAt(t, from);
        ctx.beginPath();
        ctx.moveTo(p[0], p[1]);
        for (var i = 1; i < t.lens.length; i++) {
            if (t.lens[i] > from && t.lens[i] < to) ctx.lineTo(t.pts[i][0], t.pts[i][1]);
        }
        p = pointAt(t, to);
        ctx.lineTo(p[0], p[1]);
        ctx.stroke();
    }

    /* ---------- Waveforms ---------- */
    function pressure(t) {       // ventilator pressure: fast rise, plateau, exponential release
        var p = (t % 3.2) / 3.2;
        if (p < 0.08) return p / 0.08;
        if (p < 0.38) return 0.9 + 0.1 * Math.cos((p - 0.08) * 20);
        return Math.exp(-(p - 0.38) * 14) * 0.95 + 0.05;
    }
    function ecg(t) {            // simplified P-QRS-T complex at ~72 bpm
        var p = (t % 0.85) / 0.85;
        var g = function (c, wd, a) { return a * Math.exp(-Math.pow((p - c) / wd, 2)); };
        return g(0.12, 0.035, 0.12) - g(0.27, 0.01, 0.15) + g(0.3, 0.012, 1) - g(0.33, 0.012, 0.25) + g(0.55, 0.06, 0.25);
    }

    function buildChannels() {
        var n = Math.ceil(w / WAVE_STEP) + 1;
        var defs = [
            { fn: pressure, color: function () { return accent; }, y: 0.84, amp: 0.07 },
            { fn: ecg, color: function () { return second; }, y: 0.93, amp: 0.05 }
        ];
        channels = defs.map(function (d) {
            var buf = new Float32Array(n);
            // Pre-fill so the band isn't empty on first paint.
            for (var i = 0; i < n; i++) buf[i] = d.fn((i * WAVE_STEP) / SWEEP_SPEED);
            return { fn: d.fn, color: d.color, y: d.y, amp: d.amp, buf: buf };
        });
        cursor = 0;
        clock = n * WAVE_STEP / SWEEP_SPEED;
    }

    /* ---------- Layout ---------- */
    function resize() {
        var r = hero.getBoundingClientRect();
        w = Math.max(1, Math.round(r.width));
        h = Math.max(1, Math.round(r.height));
        dpr = Math.min(window.devicePixelRatio || 1, 2);
        canvas.width = w * dpr;
        canvas.height = h * dpr;
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);

        var count = Math.max(10, Math.min(46, Math.round((w * h) / 30000)));
        traces = [];
        for (var i = 0; i < count; i++) traces.push(makeTrace());
        buildChannels();
        if (reduceMotion) draw(0);
    }

    /* ---------- Frame ---------- */
    function step(dt) {
        traces.forEach(function (t) {
            t.pos += t.speed * dt;
            if (t.flash > 0) t.flash = Math.max(0, t.flash - dt * 1.6);
            if (t.pos - t.tail > t.total) {
                t.flash = 1;
                t.pos = -rand(t.total * 0.5, t.total * 3);
            }
        });

        var n = channels.length ? channels[0].buf.length : 0;
        var advance = SWEEP_SPEED * dt / WAVE_STEP;
        var steps = Math.floor(cursor + advance) - Math.floor(cursor);
        for (var s = 0; s < steps; s++) {
            var idx = (Math.floor(cursor) + 1 + s) % n;
            clock += WAVE_STEP / SWEEP_SPEED;
            channels.forEach(function (c) { c.buf[idx] = c.fn(clock); });
        }
        cursor = (cursor + advance) % n;

        mouse.x += (mouse.tx - mouse.x) * Math.min(1, dt * 6);
        mouse.y += (mouse.ty - mouse.y) * Math.min(1, dt * 6);
    }

    function draw() {
        ctx.clearRect(0, 0, w, h);
        var boost = light ? 1.5 : 1;

        // Oscilloscope grid
        ctx.lineWidth = 1;
        ctx.strokeStyle = rgba(accent, 0.035 * boost);
        ctx.beginPath();
        for (var gx = 0; gx <= w; gx += CELL) { ctx.moveTo(gx + 0.5, 0); ctx.lineTo(gx + 0.5, h); }
        for (var gy = 0; gy <= h; gy += CELL) { ctx.moveTo(0, gy + 0.5); ctx.lineTo(w, gy + 0.5); }
        ctx.stroke();

        // Mouse spotlight
        if (mouse.x > -500) {
            var g = ctx.createRadialGradient(mouse.x, mouse.y, 0, mouse.x, mouse.y, 260);
            g.addColorStop(0, rgba(accent, 0.12 * boost));
            g.addColorStop(1, rgba(accent, 0));
            ctx.fillStyle = g;
            ctx.fillRect(mouse.x - 260, mouse.y - 260, 520, 520);
        }

        // Trace copper + pads
        ctx.lineCap = 'round';
        ctx.lineJoin = 'round';
        traces.forEach(function (t) {
            ctx.lineWidth = 1.2;
            ctx.strokeStyle = rgba(accent, 0.11 * t.fade * boost);
            strokeBetween(t, 0, t.total);
            var end = t.pts[t.pts.length - 1], start = t.pts[0];
            ctx.fillStyle = rgba(accent, (0.18 + t.flash * 0.6) * t.fade * boost);
            ctx.beginPath(); ctx.arc(end[0], end[1], 2.6 + t.flash * 2.5, 0, Math.PI * 2); ctx.fill();
            ctx.strokeStyle = rgba(accent, 0.2 * t.fade * boost);
            ctx.beginPath(); ctx.arc(start[0], start[1], 3, 0, Math.PI * 2); ctx.stroke();
        });

        // Signal pulses
        ctx.shadowColor = rgba(accent, 0.9);
        ctx.shadowBlur = 10;
        traces.forEach(function (t) {
            if (t.pos <= 0) return;
            ctx.lineWidth = 2;
            ctx.strokeStyle = rgba(accent, 0.25 * t.fade * boost);
            strokeBetween(t, t.pos - t.tail, t.pos - t.tail * 0.4);
            ctx.strokeStyle = rgba(accent, 0.75 * t.fade * boost);
            strokeBetween(t, t.pos - t.tail * 0.4, t.pos);
            if (t.pos < t.total) {
                var p = pointAt(t, t.pos);
                ctx.fillStyle = rgba(accent, t.fade);
                ctx.beginPath(); ctx.arc(p[0], p[1], 2.2, 0, Math.PI * 2); ctx.fill();
            }
        });

        // Waveforms: brightest at the sweep cursor, fading behind it, with an erase gap ahead
        channels.forEach(function (c) {
            var n = c.buf.length, cur = Math.floor(cursor), gap = 14;
            var base = h * c.y, amp = h * c.amp, col = c.color();
            ctx.lineWidth = 1.8;
            ctx.shadowColor = rgba(col, 0.8);
            for (var chunk = 0; chunk < n - 1; chunk += 6) {
                var behind = (cur - chunk + n) % n;
                if (behind > n - gap) continue;
                var xFade = 0.3 + 0.7 * (chunk / n);
                var a = (0.15 + 0.6 * Math.pow(1 - behind / n, 2)) * xFade * boost;
                ctx.strokeStyle = rgba(col, Math.min(1, a));
                ctx.beginPath();
                for (var i = chunk; i <= Math.min(chunk + 6, n - 1); i++) {
                    var b = (cur - i + n) % n;
                    if (b > n - gap) break;
                    var px = i * WAVE_STEP, py = base - c.buf[i] * amp;
                    if (i === chunk) ctx.moveTo(px, py); else ctx.lineTo(px, py);
                }
                ctx.stroke();
            }
            // Sweep head
            var hx = cur * WAVE_STEP, hy = base - c.buf[cur] * amp;
            ctx.fillStyle = rgba(col, 0.95);
            ctx.beginPath(); ctx.arc(hx, hy, 2.6, 0, Math.PI * 2); ctx.fill();
        });
        ctx.shadowBlur = 0;
    }

    function frame(now) {
        rafId = 0;
        var dt = lastTime ? Math.min(0.05, (now - lastTime) / 1000) : 0.016;
        lastTime = now;
        step(dt);
        draw();
        schedule();
    }

    function schedule() {
        if (!rafId && visible && !document.hidden && !reduceMotion) rafId = requestAnimationFrame(frame);
    }

    /* ---------- Wiring ---------- */
    readTheme();
    resize();

    if ('ResizeObserver' in window) new ResizeObserver(resize).observe(hero);
    else window.addEventListener('resize', resize);

    new MutationObserver(function () { readTheme(); if (reduceMotion) draw(); })
        .observe(root, { attributes: true, attributeFilter: ['data-theme'] });

    if ('IntersectionObserver' in window) {
        new IntersectionObserver(function (entries) {
            visible = entries[0].isIntersecting;
            lastTime = 0;
            schedule();
        }).observe(hero);
    }
    document.addEventListener('visibilitychange', function () { lastTime = 0; schedule(); });

    hero.addEventListener('pointermove', function (e) {
        var r = hero.getBoundingClientRect();
        mouse.tx = e.clientX - r.left;
        mouse.ty = e.clientY - r.top;
        if (mouse.x < -500) { mouse.x = mouse.tx; mouse.y = mouse.ty; }
    });
    hero.addEventListener('pointerleave', function () { mouse.tx = mouse.ty = -1000; mouse.x = mouse.y = -1000; });

    schedule();

    /* ---------- Typed role line ---------- */
    var typed = document.querySelector('.hero .typed');
    if (typed && !reduceMotion) {
        var phrases;
        try { phrases = JSON.parse(typed.getAttribute('data-phrases')); } catch (e) { phrases = null; }
        if (phrases && phrases.length > 1) {
            var pi = 0, ci = phrases[0].length, deleting = false;
            var tick = function () {
                if (deleting) {
                    ci--;
                    if (ci <= 0) { deleting = false; pi = (pi + 1) % phrases.length; }
                } else {
                    ci++;
                }
                typed.textContent = phrases[pi].slice(0, Math.max(0, ci));
                var delay = deleting ? 28 : 55;
                if (!deleting && ci >= phrases[pi].length) { deleting = true; delay = 2200; }
                else if (!deleting && ci === 0) delay = 300;
                setTimeout(tick, delay);
            };
            setTimeout(function () { deleting = true; tick(); }, 3200);
        }
    }
})();
