set PATH=%PATH%;C:\Qt\5.12.0\msvc2017_64\bin;
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
C:\Qt\5.12.0\msvc2017_64\bin\qmake -tp vc -r
C:\Qt\5.12.0\msvc2017_64\bin\qmake config+="debug_and_release"