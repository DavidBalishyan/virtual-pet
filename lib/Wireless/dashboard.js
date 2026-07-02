(() => {
    const host = location.hostname;
    const ws = new WebSocket('ws://' + host + ':81/');
    let reconnectTimer = null;

    const moodColors = {
        happy: '#2ecc71',
        unwell: '#9b59b6',
        hungry: '#e74c3c',
        thirsty: '#e67e22',
        neutral: '#95a5a6'
    };

    const statConfigs = [
        { key: 'fullness', label: 'Fullness', color: '#e74c3c' },
        { key: 'happy', label: 'Happy', color: '#2ecc71' },
        { key: 'energy', label: 'Energy', color: '#3498db' },
        { key: 'cleanliness', label: 'Cleanliness', color: '#1abc9c' },
        { key: 'sick', label: 'Sick', color: '#9b59b6' },
        { key: 'hydration', label: 'Hydration', color: '#e67e22' },
        { key: 'tired', label: 'Tired', color: '#95a5a6' },
        { key: 'sad', label: 'Sad', color: '#7f8c8d' }
    ];

    const setConnected = (ok) => {
        const led = document.getElementById('statusLed');
        const txt = document.getElementById('wsStatus');
        if (ok) {
            led.style.background = '#2ecc71';
            txt.textContent = 'connected';
        } else {
            led.style.background = '#e74c3c';
            txt.textContent = 'disconnected';
        }
    };

    ws.onopen = () => {
        setConnected(true);
    };

    ws.onclose = () => {
        setConnected(false);
        if (reconnectTimer) clearTimeout(reconnectTimer);
        reconnectTimer = setTimeout(() => {
            location.reload();
        }, 3000);
    };

    ws.onmessage = (e) => {
        const data = JSON.parse(e.data);
        document.getElementById('petName').textContent = data.pet.name;
        const mb = document.getElementById('moodBadge');
        const m = data.pet.mood;
        mb.textContent = m.charAt(0).toUpperCase() + m.slice(1);
        mb.style.background = moodColors[m] || '#95a5a6';
        const dn = document.getElementById('deathNotice');
        dn.style.display = data.pet.alive ? 'none' : 'block';
        const sc = document.getElementById('stats');
        sc.innerHTML = '';
        for (let i = 0; i < statConfigs.length; i++) {
            const s = statConfigs[i];
            let v = data.stats[s.key];
            if (v < 0) v = 0;
            if (v > 100) v = 100;
            const d = document.createElement('div');
            d.className = 'stat';
            d.innerHTML =
                '<div class="stat-label">' +
                '<span class="stat-name">' + s.label + '</span>' +
                '<span class="stat-value">' + data.stats[s.key] + '</span>' +
                '</div>' +
                '<div class="bar-bg">' +
                '<div class="bar-fill" style="width:' + v + '%;background:' + s.color + ';"></div>' +
                '</div>';
            sc.appendChild(d);
        }
    };
})();
