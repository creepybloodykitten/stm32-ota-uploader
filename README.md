# spot firmware update system of stm32 everywhere with connection to host raspberry pi 4

steps:
(temporary decision using virtual network)  
1)download zerotier on sender and receiver  
2)establish ssh key (for automatization without password)  
2.1)make shortcut of .ps1 script with
"powershell.exe -NoProfile -ExecutionPolicy Bypass -File "D:\your\path\to\sending_script.ps1""
3)download stm32flash on raspberry pi  
4)physically connect stm to raspberry pi
