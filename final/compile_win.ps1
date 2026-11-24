Get-Item -LiteralPath build -ErrorAction SilentlyContinue | Remove-Item -Recurse

mkdir build
Set-Location build

cmake -G "MinGW Makefiles" ..

cmake --build . --config Release

Set-Location ..
