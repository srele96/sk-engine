param (
  [Parameter(Mandatory = $true)]
  [string[]]$files
)

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
