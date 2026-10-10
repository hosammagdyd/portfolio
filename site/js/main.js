(function () {
    'use strict';

    /* ---------- Theme toggle ---------- */
    var root = document.documentElement;
    var themeToggle = document.querySelector('.theme-toggle');
    var STORAGE_KEY = 'portfolio-theme';

    function applyTheme(theme) {
        root.setAttribute('data-theme', theme);
        try { localStorage.setItem(STORAGE_KEY, theme); } catch (e) { /* storage unavailable */ }
    }

    (function initTheme() {
        var stored = null;
        try { stored = localStorage.getItem(STORAGE_KEY); } catch (e) { /* storage unavailable */ }
        if (stored === 'light' || stored === 'dark') {
            applyTheme(stored);
        } else if (window.matchMedia && window.matchMedia('(prefers-color-scheme: light)').matches) {
            applyTheme('light');
        }
    })();

    if (themeToggle) {
        themeToggle.addEventListener('click', function () {
            var current = root.getAttribute('data-theme') === 'light' ? 'light' : 'dark';
            applyTheme(current === 'light' ? 'dark' : 'light');
        });
    }

    /* ---------- Mobile nav toggle ---------- */
    var navToggle = document.querySelector('.nav-toggle');
    var navLinks = document.querySelector('.nav-links');

    if (navToggle && navLinks) {
        navToggle.addEventListener('click', function () {
            var isOpen = navLinks.classList.toggle('is-open');
            navToggle.setAttribute('aria-expanded', String(isOpen));
        });

        document.addEventListener('keydown', function (e) {
            if (e.key === 'Escape' && navLinks.classList.contains('is-open')) {
                navLinks.classList.remove('is-open');
                navToggle.setAttribute('aria-expanded', 'false');
                navToggle.focus();
            }
        });

        navLinks.querySelectorAll('a').forEach(function (link) {
            link.addEventListener('click', function () {
                navLinks.classList.remove('is-open');
                navToggle.setAttribute('aria-expanded', 'false');
            });
        });
    }

    /* ---------- Scroll-reveal ---------- */
    document.querySelectorAll('.stagger').forEach(function (group) {
        Array.prototype.forEach.call(group.children, function (child, i) {
            child.style.setProperty('--i', i);
        });
    });

    var revealTargets = document.querySelectorAll('.reveal, .stagger');
    if ('IntersectionObserver' in window && revealTargets.length) {
        var revealObserver = new IntersectionObserver(function (entries) {
            entries.forEach(function (entry) {
                if (entry.isIntersecting) {
                    entry.target.classList.add('is-visible');
                    revealObserver.unobserve(entry.target);
                }
            });
        }, { threshold: 0.05, rootMargin: '0px 0px -40px 0px' });
        revealTargets.forEach(function (el) { revealObserver.observe(el); });
    } else {
        revealTargets.forEach(function (el) { el.classList.add('is-visible'); });
    }

    /* ---------- Active nav link on scroll ---------- */
    var sections = document.querySelectorAll('main section[id]');
    var navAnchors = document.querySelectorAll('.nav-links a[href^="#"]');
    if ('IntersectionObserver' in window && sections.length && navAnchors.length) {
        var navObserver = new IntersectionObserver(function (entries) {
            entries.forEach(function (entry) {
                if (!entry.isIntersecting) return;
                navAnchors.forEach(function (a) { a.classList.remove('active-link'); });
                var match = document.querySelector('.nav-links a[href="#' + entry.target.id + '"]');
                if (match) match.classList.add('active-link');
            });
        }, { rootMargin: '-45% 0px -50% 0px' });
        sections.forEach(function (s) { navObserver.observe(s); });
    }

    /* ---------- Scroll-driven effects ---------- */
    var siteNav = document.querySelector('.site-nav');
    var heroContent = document.querySelector('.hero-content');
    var timeline = document.querySelector('.timeline');
    var timelineItems = document.querySelectorAll('.timeline-item');
    var backToTop = document.querySelector('.back-to-top');
    var reduceMotion = window.matchMedia && window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    var ticking = false;

    function clamp01(v) { return v < 0 ? 0 : v > 1 ? 1 : v; }

    function onScroll() {
        ticking = false;
        var y = window.scrollY;
        var vh = window.innerHeight;
        var maxScroll = document.documentElement.scrollHeight - vh;

        root.style.setProperty('--scroll', maxScroll > 0 ? (y / maxScroll).toFixed(4) : 0);
        if (siteNav) siteNav.classList.toggle('is-scrolled', y > 10);
        if (backToTop) backToTop.classList.toggle('is-shown', y > vh * 0.8);

        if (heroContent && !reduceMotion) {
            heroContent.style.setProperty('--hero-p', clamp01(y / (vh * 0.9)).toFixed(4));
        }

        if (timeline) {
            // Fill the line up to the point 60% down the viewport.
            var rect = timeline.getBoundingClientRect();
            var marker = vh * 0.6;
            timeline.style.setProperty('--line', clamp01((marker - rect.top) / rect.height).toFixed(4));
            timelineItems.forEach(function (item) {
                item.classList.toggle('is-active', item.getBoundingClientRect().top < marker);
            });
        }
    }

    window.addEventListener('scroll', function () {
        if (!ticking) { ticking = true; window.requestAnimationFrame(onScroll); }
    }, { passive: true });
    window.addEventListener('resize', onScroll);
    onScroll();

    if (backToTop) {
        backToTop.addEventListener('click', function () {
            window.scrollTo({ top: 0, behavior: reduceMotion ? 'auto' : 'smooth' });
        });
    }

    /* ---------- Footer year ---------- */
    var yearEl = document.getElementById('year');
    if (yearEl) yearEl.textContent = new Date().getFullYear();

    /* ---------- Lazy-load the Qt WASM demo ---------- */
    var launchBtn = document.getElementById('launch-demo');
    var demoStage = document.getElementById('demo-stage');

    if (launchBtn && demoStage) {
        launchBtn.addEventListener('click', function () {
            if (demoStage.classList.contains('is-loading') || demoStage.classList.contains('is-loaded')) return;
            demoStage.classList.add('is-loading');
            launchBtn.disabled = true;
            launchBtn.textContent = 'Loading demo...';

            var iframe = document.createElement('iframe');
            iframe.title = 'Qt Showcase - Ventilator Monitor live demo';
            iframe.setAttribute('allow', 'fullscreen');
            iframe.addEventListener('load', function () {
                demoStage.classList.remove('is-loading');
                demoStage.classList.add('is-loaded');
                launchBtn.style.display = 'none';
            });
            iframe.src = 'demo/appQtShowcase.html';
            demoStage.appendChild(iframe);
        });
    }
})();
