#!/usr/bin/pwsh
[CmdletBinding(DefaultParameterSetName = 'TARGET')]
param (
    [Parameter(ParameterSetName = 'ALL')]
    [Switch]
    $All,
    [Parameter()]
    [Switch]
    $Dbg,
    [Parameter()]
    [Switch]
    $Run,
    [Parameter(ParameterSetName = 'TARGET')]
    [ValidateSet('UfWebCameraV4L', 'UfWebCameraV4L_TEST')]
    [string[]]
    $Target = @('UfWebCameraV4L')
)
try {
    if (-not (Test-Path "$PSScriptRoot/build")) {
        New-Item -ItemType Directory "$PSScriptRoot/build" -Force
    }
    Push-Location "$PSScriptRoot/build"
    $type = if ($Dbg.IsPresent) { 'DEBUG' }else { 'RELEASE' }
    $params = @(
        '--build'
        '.'
        '--config'
        $type
    )
    if (!$All.IsPresent) {
        $params += @('--target')
        foreach ($t in $Target) {
            $params += @($t)
        }
    }
    Write-Host -NoNewline 'Building with: '
    Write-Host ($params -join ' ')
    cmake .. && cmake @params
    if ($Run.IsPresent -and ($all.ispresent || $target -contains 'UfWebCameraV4L_TEST')) {
       & ./UfWebCameraV4L
    }
} finally {
    Pop-Location
}