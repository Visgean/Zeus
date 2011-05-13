@echo off
cd make
"..\bin\php\php.exe" -q make.php -b "default" "Hello"
cd ..
