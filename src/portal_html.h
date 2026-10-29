#pragma once
#include <Arduino.h>

const char PORTAL_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Wi-Fi Setup</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background: #121212; color: #e0e0e0; display: flex; justify-content: center; align-items: center; min-height: 100vh; padding: 15px; }
    .card { background: #1e1e1e; border-radius: 14px; padding: 22px; width: 100%; max-width: 360px; box-shadow: 0 8px 24px rgba(0,0,0,0.5); }
    h2 { font-size: 1.25rem; margin-bottom: 16px; text-align: center; color: #fff; font-weight: 600; }
    .head-bar { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; }
    .label { font-size: 0.85rem; color: #aaa; }
    .btn-icon { background: transparent; border: none; color: #4da3ff; font-size: 1.2rem; cursor: pointer; padding: 2px 6px; }
    .net-list { max-height: 140px; overflow-y: auto; margin-bottom: 14px; border: 1px solid #333; border-radius: 8px; background: #181818; }
    .net-item { display: flex; justify-content: space-between; align-items: center; padding: 10px 12px; cursor: pointer; border-bottom: 1px solid #282828; font-size: 0.95rem; }
    .net-item:last-child { border-bottom: none; }
    .net-item:hover, .net-item:active { background: #282828; }
    .net-meta { font-size: 0.8rem; color: #888; display: flex; align-items: center; gap: 4px; }
    .form-group { margin-bottom: 12px; }
    input { width: 100%; padding: 11px 12px; background: #2a2a2a; border: 1px solid #444; border-radius: 8px; color: #fff; font-size: 0.95rem; outline: none; }
    input:focus { border-color: #4da3ff; }
    .pass-wrap { position: relative; }
    .pass-wrap input { padding-right: 40px; }
    .toggle-pass { position: absolute; right: 8px; top: 50%; transform: translateY(-50%); background: none; border: none; color: #888; cursor: pointer; font-size: 1.1rem; padding: 4px; }
    .btn-submit { width: 100%; padding: 12px; background: #0070f3; border: none; border-radius: 8px; color: #fff; font-size: 1rem; font-weight: 600; cursor: pointer; margin-top: 4px; }
    .btn-submit:active { background: #0060df; }
    .btn-submit:disabled { background: #444; color: #888; cursor: not-allowed; }
    .empty { padding: 14px; text-align: center; color: #777; font-size: 0.85rem; }
    .status { text-align: center; font-size: 0.9rem; margin-top: 14px; color: #4da3ff; display: none; line-height: 1.4; }
  </style>
</head>
<body>
  <div class="card">
    <h2>Wi-Fi Setup</h2>
    <div class="head-bar">
      <span class="label">Networks</span>
      <button class="btn-icon" id="scan-btn" onclick="scan()" title="Rescan">&#x21bb;</button>
    </div>
    <div id="networks" class="net-list">
      <div class="empty">Scanning...</div>
    </div>
    <form id="wf" onsubmit="save(event)">
      <div class="form-group">
        <input type="text" id="s" name="ssid" placeholder="Network Name" required autocomplete="off">
      </div>
      <div class="form-group pass-wrap">
        <input type="password" id="p" name="password" placeholder="Password" autocomplete="current-password">
        <button type="button" class="toggle-pass" onclick="togglePass()">&#x1f441;</button>
      </div>
      <button type="submit" id="btn" class="btn-submit">Connect</button>
    </form>
    <div id="status" class="status"></div>
  </div>
  <script>
    function scan() {
      const el = document.getElementById('networks');
      el.innerHTML = '<div class="empty">Scanning...</div>';
      fetch('/scan').then(r => r.json()).then(data => {
        if (!data || data.length === 0) {
          el.innerHTML = '<div class="empty">No networks found</div>';
          return;
        }
        el.innerHTML = '';
        data.forEach(n => {
          const item = document.createElement('div');
          item.className = 'net-item';
          const pct = Math.min(Math.max(2 * (n.rssi + 100), 0), 100);
          item.innerHTML = '<span>' + n.ssid + '</span><span class="net-meta">' + (n.enc ? '&#x1f512; ' : '') + pct + '%</span>';
          item.onclick = () => {
            document.getElementById('s').value = n.ssid;
            document.getElementById('p').focus();
          };
          el.appendChild(item);
        });
      }).catch(() => {
        el.innerHTML = '<div class="empty">Scan failed. Click &#x21bb;</div>';
      });
    }
    function togglePass() {
      const p = document.getElementById('p');
      p.type = (p.type === 'password') ? 'text' : 'password';
    }
    function save(e) {
      e.preventDefault();
      const s = document.getElementById('s').value.trim();
      const p = document.getElementById('p').value;
      if (!s) return;
      document.getElementById('btn').disabled = true;
      const st = document.getElementById('status');
      st.style.display = 'block';
      st.innerText = 'Connecting to ' + s + '...';
      const params = new URLSearchParams();
      params.append('ssid', s);
      params.append('password', p);
      fetch('/save', {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
        body: params.toString()
      }).then(() => {
        st.innerText = 'Connecting to ' + s + '... You can close this window.';
      }).catch(() => {
        st.innerText = 'Connecting to ' + s + '... You can close this window.';
      });
    }
    window.onload = scan;
  </script>
</body>
</html>)rawliteral";

