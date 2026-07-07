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

  // WebSocket helpers
  const sendCommand = (obj) => {
    if (ws.readyState !== WebSocket.OPEN) return;
    ws.send(JSON.stringify(obj));
  };

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

  // action buttons
  document.querySelectorAll('[data-action]').forEach((btn) => {
    btn.addEventListener('click', () => {
      const action = btn.getAttribute('data-action');
      if (action === 'reset' && !confirm('Reset the pet to default? All progress will be lost.')) {
        return;
      }
      sendCommand({ action: action });
    });
  });

  // rename
  const nameInput = document.getElementById('nameInput');
  const renameBtn = document.getElementById('renameBtn');

  const doRename = () => {
    const name = nameInput.value.trim();
    if (name.length === 0) return;
    sendCommand({ action: 'setName', name: name });
    // Re-accept server sync after rename so the confirmed name shows up
    delete nameInput.dataset.userEdited;
  };

  nameInput.addEventListener('input', () => {
    nameInput.dataset.userEdited = '1';
  });

  renameBtn.addEventListener('click', doRename);
  nameInput.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') doRename();
  });

  // brightness slider
  const brightnessSlider = document.getElementById('brightnessSlider');
  const brightnessValue = document.getElementById('brightnessValue');

  // Send the new level as the slider moves. We mark the slider as being dragged
  // so incoming stats (which carry the device's brightness) don't yank the
  // handle out from under the user's finger mid-drag.
  brightnessSlider.addEventListener('input', () => {
    const v = parseInt(brightnessSlider.value, 10);
    brightnessValue.textContent = v + '%';
    brightnessSlider.dataset.userEditing = '1';
    sendCommand({ action: 'setBrightness', value: v });
  });
  // Once the drag ends, re-accept server sync so the confirmed value shows.
  brightnessSlider.addEventListener('change', () => {
    delete brightnessSlider.dataset.userEditing;
  });

  // WebSocket lifecycle
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

  // Incoming stats
  ws.onmessage = (e) => {
    const data = JSON.parse(e.data);

    // Update title
    document.getElementById('petName').textContent = data.pet.name;
    // Sync the name input field too (so it shows the current name on load)
    if (nameInput && !nameInput.dataset.userEdited) {
      nameInput.value = data.pet.name;
    }

    // Update mood badge
    const mb = document.getElementById('moodBadge');
    const m = data.pet.mood;
    mb.textContent = m.charAt(0).toUpperCase() + m.slice(1);
    mb.style.background = moodColors[m] || '#95a5a6';

    // Sync the brightness slider to the device's level, unless the user is
    // mid-drag (in which case their value takes precedence).
    if (typeof data.brightness === 'number' && !brightnessSlider.dataset.userEditing) {
      brightnessSlider.value = data.brightness;
      brightnessValue.textContent = data.brightness + '%';
    }

    // Death notice
    const dn = document.getElementById('deathNotice');
    dn.style.display = data.pet.alive ? 'none' : 'block';

    // Toggle action section — disable buttons when dead
    const actionBtns = document.querySelectorAll('.action-btn');
    actionBtns.forEach((b) => {
      const a = b.getAttribute('data-action');
      // Allow reset even when dead; disable care actions
      if (a !== 'reset') {
        b.disabled = !data.pet.alive;
      }
    });

    // Stat bars
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
