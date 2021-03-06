; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{84AF32E8-9487-421E-9A8B-73712049EBBD}
AppName=CSCHEM
AppVersion=0.1.2
AppPublisher=Daniel Wagenaar
AppPublisherURL=http://www.danielwagenaar.net
AppSupportURL=http://www.danielwagenaar.net
AppUpdatesURL=http://www.danielwagenaar.net
DefaultDirName={pf}\CSCHEM
DisableProgramGroupPage=yes
LicenseFile=C:\Users\Daniel Wagenaar\Documents\Progs\cschem\GPL-3.0.txt
OutputDir=..\..\releases
OutputBaseFilename=cschem-0.1.2-x86-setup
SetupIconFile=C:\Users\Daniel Wagenaar\Documents\Progs\cschem\src\App\cschem.ico
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "C:\Users\Daniel Wagenaar\Documents\Progs\cschem\release-cschem-x86\*"; DestDir: "{app}"; Flags: ignoreversion  recursesubdirs createallsubdirs

[Icons]
Name: "{commonprograms}\CSCHEM"; Filename: "{app}\cschem.exe"
Name: "{commondesktop}\CSCHEM"; Filename: "{app}\cschem.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\cschem.exe"; Description: "{cm:LaunchProgram,CSCHEM}"; Flags: nowait postinstall skipifsilent

