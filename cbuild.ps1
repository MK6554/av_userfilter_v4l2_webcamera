#!/usr/bin/pwsh
[CmdletBinding(DefaultParameterSetName = 'TARGET')]
param (
    [Parameter(ParameterSetName = 'ALL')]
    [Switch]
    $All,
    [Parameter()]
    [Switch]
    $Dbg,
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
    $type = if ($Dbg.IsPresent) { 'Debug' }else { 'Release' }
    $generateParams=@(
        ".."
        "-DCMAKE_BUILD_TYPE=$type"
    )
    $buildParams = @(
        '--build'
        '.'
        '--config'
        $type.ToUpper()
    )
    if (!$All.IsPresent) {
        $buildParams += @('--target')
        foreach ($t in $Target) {
            $buildParams += @($t)
        }
    }
    Write-Host "=============`nGenerating with: " -ForeGround Cyan
    $generado = "'" + ($generateParams -join "', '")
    Write-Host $generado -ForeGround Cyan
    cmake @generateParams
    if($LASTEXITCODE -ne 0){
        Write-Error "Could not generate build files!"
        exit $LASTEXITCODE
    }
    Write-Host "Generated successfully`n=============" -ForeGround Cyan
    Write-Host "=============`nBuilding with: " -ForeGround Green
    $buildado = "'" + ($buildParams -join "', '")
    Write-Host $buildado -ForeGround Green
    cmake @buildParams
    if($LASTEXITCODE -ne 0){
        Write-Error "Could not build!"
        exit $LASTEXITCODE
    }
    Write-Host "Built successfully`n=============" -ForeGround Green
} catch {
    exit 1
} finally {
    Pop-Location
}