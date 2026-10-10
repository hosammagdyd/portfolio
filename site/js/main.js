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

        navLinks.querySelectorAll('a').forEach(function (link) {
            link.addEventListener('click', function () {
                navLinks.classList.remove('is-open');
                navToggle.setAttribute('aria-expanded', 'false');
            });
        });
    }

    /* ---------- Scroll-reveal ---------- */
    var revealTargets = document.querySelectorAll('.reveal');
    if ('IntersectionObserver' in window && revealTargets.length) {
        var revealObserver = new IntersectionObserver(function (entries) {
            entries.forEach(function (entry) {
                if (entry.isIntersecting) {
                    entry.target.classList.add('is-visible');
                    revealObserver.unobserve(entry.target);
                }
            });
        }, { threshold: 0.15 });
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
