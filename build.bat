rc.exe /v src/version.rc
cl.exe /c /O1 /GL /GS- /MT src/proxy.c /Fo:proxy.obj
link.exe /DLL /OUT:version.dll /ENTRY:DllMain /NODEFAULTLIB /SUBSYSTEM:WINDOWS ^
/LTCG /OPT:REF /OPT:ICF ^
proxy.obj src/version.res kernel32.lib user32.lib