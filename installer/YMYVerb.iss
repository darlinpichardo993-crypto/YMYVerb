#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif

#define MyAppName "YMYVerb"
#define MyPublisher "YMY Records / ORYMY"

[Setup]
AppId={{B874CC36-5C0B-4FB1-9F89-8DC3B61A7F02}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyPublisher}
DefaultDirName={autopf}\YMY Records\YMYVerb
DefaultGroupName=YMY Records\YMYVerb
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=YMYVerb-Setup
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=no
RestartApplications=no

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\dist\YMYVerb.vst3\*"; DestDir: "{commoncf}\VST3\YMYVerb.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\README-ES.md"; DestDir: "{app}"; DestName: "LEEME-ANTES-DE-USAR.txt"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\YMY Records\YMYVerb - notas"; Filename: "{app}\LEEME-ANTES-DE-USAR.txt"

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf}\VST3\YMYVerb.vst3"

[Code]
function InitializeSetup(): Boolean;
begin
  Result := True;
  if not IsWin64 then
  begin
    MsgBox('YMYVerb requiere Windows de 64 bits.', mbError, MB_OK);
    Result := False;
  end;
end;
