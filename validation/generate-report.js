#!/usr/bin/env node
// Generates validation-report.html from validation-results.json.
// Screenshot img/link paths are relative to the report file's own location (shots/<name>.png).
const fs = require('fs');
const path = require('path');

const dir = __dirname;
const jsonPath = path.join(dir, 'validation-results.json');
const data = JSON.parse(fs.readFileSync(jsonPath, 'utf8'));

const esc = (s) => String(s == null ? '' : s)
  .replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
  .replace(/"/g, '&quot;').replace(/'/g, '&#39;');

const overall = String(data.overall || 'fail').toLowerCase();
const bannerClass = overall === 'pass' ? 'banner pass' : 'banner fail';
const bannerText = overall === 'pass'
  ? 'OVERALL VERDICT: PASS'
  : 'OVERALL VERDICT: FAIL';

const metricsHtml = (m) => {
  if (!m || Object.keys(m).length === 0) return '<span class="muted">none</span>';
  return '<ul class="metrics">' + Object.entries(m).map(([k, v]) =>
    `<li><code>${esc(k)}</code> = ${esc(typeof v === 'object' ? JSON.stringify(v) : v)}</li>`
  ).join('') + '</ul>';
};

const rows = (data.steps || []).map((st, i) => {
  const res = String(st.result || '').toLowerCase();
  const resClass = res === 'pass' ? 'pass' : (res === 'blocked' ? 'blocked' : 'fail');
  const shot = st.screenshotPath
    ? `<a href="${esc(st.screenshotPath)}"><img src="${esc(st.screenshotPath)}" alt="${esc(st.id)} screenshot" loading="lazy"></a>`
    : '<span class="muted">no screenshot (not applicable)</span>';
  return `
  <tr class="step-row" id="step-${esc(st.id)}">
    <td class="num">${i + 1}</td>
    <td><div class="sid">${esc(st.id)}</div><div class="sname">${esc(st.name)}</div></td>
    <td class="action">${esc(st.action)}</td>
    <td><span class="result ${resClass}">${esc(res.toUpperCase())}</span></td>
    <td class="context">${esc(st.context)}</td>
    <td class="shot">${shot}${st.screenshotPath ? '' : ''}</td>
    <td class="metrics-cell">${metricsHtml(st.metrics)}</td>
  </tr>`;
}).join('\n');

const shotCount = (data.steps || []).filter(s => s.screenshotPath).length;

const html = `<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Ditto Validation Report — ${esc(data.runId)}</title>
<style>
  :root { color-scheme: light; }
  body { font-family: 'Segoe UI', system-ui, sans-serif; margin: 0; background: #f5f6f8; color: #1c1c1c; }
  header { background: #1f2937; color: #fff; padding: 24px 32px; }
  header h1 { margin: 0 0 4px 0; font-size: 22px; }
  header .sub { color: #cbd5e1; font-size: 13px; }
  .banner { padding: 18px 32px; font-size: 20px; font-weight: 700; letter-spacing: 0.5px; }
  .banner.pass { background: #16a34a; color: #fff; }
  .banner.fail { background: #dc2626; color: #fff; }
  .meta { padding: 16px 32px; background: #fff; border-bottom: 1px solid #e5e7eb; }
  .meta table { border-collapse: collapse; font-size: 13px; }
  .meta td { padding: 3px 14px 3px 0; }
  .meta td.k { color: #64748b; }
  code.hash { font-size: 12px; word-break: break-all; background: #f1f5f9; padding: 2px 6px; border-radius: 4px; }
  .wrap { padding: 20px 32px 40px 32px; }
  table.steps { width: 100%; border-collapse: collapse; background: #fff; box-shadow: 0 1px 3px rgba(0,0,0,.08); font-size: 13px; }
  table.steps th { background: #334155; color: #fff; text-align: left; padding: 10px 12px; position: sticky; top: 0; }
  table.steps td { border-bottom: 1px solid #e5e7eb; padding: 12px; vertical-align: top; }
  td.num { color: #94a3b8; font-weight: 600; }
  .sid { font-family: Consolas, monospace; font-weight: 700; font-size: 12px; color: #0f172a; }
  .sname { font-weight: 600; margin-top: 2px; }
  .result { display: inline-block; padding: 3px 10px; border-radius: 12px; font-weight: 700; font-size: 11px; }
  .result.pass { background: #dcfce7; color: #15803d; }
  .result.fail { background: #fee2e2; color: #b91c1c; }
  .result.blocked { background: #fef3c7; color: #b45309; }
  td.context { max-width: 520px; line-height: 1.45; color: #334155; }
  td.action { max-width: 240px; color: #475569; }
  td.shot img { max-width: 220px; max-height: 160px; border: 1px solid #cbd5e1; border-radius: 4px; display: block; }
  td.shot a { font-size: 11px; color: #2563eb; }
  ul.metrics { margin: 0; padding-left: 16px; font-size: 12px; }
  ul.metrics li { margin-bottom: 2px; word-break: break-word; }
  ul.metrics code { background: #f1f5f9; padding: 1px 4px; border-radius: 3px; }
  .muted { color: #94a3b8; font-style: italic; }
  .summary { background: #fff; border-left: 5px solid #334155; padding: 14px 18px; margin: 18px 0; font-size: 13px; line-height: 1.5; box-shadow: 0 1px 3px rgba(0,0,0,.08); }
  footer { padding: 12px 32px; color: #94a3b8; font-size: 11px; }
</style>
</head>
<body>
<header>
  <h1>Ditto Validation Report — ${esc(data.runId)}</h1>
  <div class="sub">Validation plan revision ${esc(data.validationPlanRevision)} — Preserve custom clip title when editing content by default (+ double-click tab rename)</div>
</header>
<div class="${bannerClass}">${bannerText}</div>
<div class="meta">
  <table>
    <tr><td class="k">Run ID</td><td>${esc(data.runId)}</td></tr>
    <tr><td class="k">Started (UTC)</td><td>${esc(data.startedUtc)}</td></tr>
    <tr><td class="k">Finished (UTC)</td><td>${esc(data.finishedUtc)}</td></tr>
    <tr><td class="k">EXE path</td><td>${esc(data.exePath)}</td></tr>
    <tr><td class="k">EXE SHA256</td><td><code class="hash">${esc(data.exeSha256)}</code></td></tr>
    <tr><td class="k">Steps</td><td>${(data.steps || []).length} (${shotCount} with screenshots)</td></tr>
    <tr><td class="k">Plan</td><td>validation-plan.md (revision ${esc(data.validationPlanRevision)})</td></tr>
  </table>
</div>
<div class="wrap">
  <div class="summary"><strong>Summary:</strong> ${esc(data.summary)}</div>
  <table class="steps">
    <thead>
      <tr><th>#</th><th>Step</th><th>Action</th><th>Result</th><th>Context / Evidence</th><th>Screenshot</th><th>Metrics</th></tr>
    </thead>
    <tbody>${rows}</tbody>
  </table>
</div>
<footer>Generated programmatically from validation-results.json — screenshot paths are relative to this report file (shots/&lt;name&gt;.png). Upstream issue: sabrogden/Ditto#1167. Branch: havra/preserve-custom-clip-title.</footer>
</body>
</html>`;

const outPath = path.join(dir, 'validation-report.html');
fs.writeFileSync(outPath, html, 'utf8');
console.log('WROTE ' + outPath + ' (' + html.length + ' bytes)');

// self-check: every referenced screenshot must exist relative to the report dir
let missing = 0;
for (const st of (data.steps || [])) {
  if (st.screenshotPath) {
    const p = path.join(dir, st.screenshotPath);
    if (!fs.existsSync(p)) { console.error('MISSING SCREENSHOT: ' + p); missing++; }
  }
}
console.log(missing === 0 ? 'ALL SCREENSHOT PATHS OK' : (missing + ' MISSING PATHS'));
