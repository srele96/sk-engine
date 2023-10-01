param (
  [Parameter(Mandatory = $true)]
  [string[]]$files
)

<#
.SYNOPSIS
Compiles a list of GLSL shader files to SPIRV-H format.

.DESCRIPTION
This function takes in a list of GLSL shader file names and generates a corresponding .spv file for each file relative to its location.

.PARAMETER files
A list of file names to be compiled to SPIR-V format.

.NOTES
File compilation is performed using the glslc.exe tool from the Vulkan SDK.
#>
function compileFiles {
  param (
    [Parameter(Mandatory = $true)]
    [string[]]$files
  )

  foreach ($file in $files) {
    $filePath = Join-Path (Get-Location).Path $file

    $outPath = "$filePath.spv"
    # https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Shader_modules
    $compile = "C:/VulkanSDK/1.3.261.1/bin/glslc.exe $filePath -o $outPath"

    Write-Host "Compiling '$filePath' to '$outPath'."

    Invoke-Expression $compile

    if ($LASTEXITCODE -eq 0) {
      Write-Host "Compilation successful."
    }

    Write-Host "----"
  }
}

compileFiles $files
