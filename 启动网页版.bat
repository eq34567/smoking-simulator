@echo off
cd /d "%~dp0"
start "" http://localhost:8765
where node >nul 2>nul && (node serve.js) || ("E:\node\node.exe" serve.js)
