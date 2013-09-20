@cd /d %~dp0
@PATH=%WINDOR%\Microsoft.NET\Framework\v4.0.30319;%PATH%
regasm /codebase /tlb:SaiHooker.tlb SaiHooker.dll /verbose %1
pause
