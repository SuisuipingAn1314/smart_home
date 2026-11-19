@echo off
chcp 65001 >nul
cd /d C:\Users\hyt\Documents\KeilDocument\My_STM32
git pull origin main
git add .
git commit -m "自动同步 %date% %time%"
git push origin main
echo 同步完成，按任意键关闭窗口
pause
