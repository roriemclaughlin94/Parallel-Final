# Parallel-Final
To run the project, please download the .rar, all files are included within the .rar, open with Visual Studio and compile.
For the serial version, copy the code from the separate main.c++ and replace with the main in the Visual Studio project.

Unfortuntely, you'll need to add a GLUT library to particular folders in order to run the program. The files are included within
the .rar with the foolder entitled GLUT.

Follow these instructions to install:



This is GLUT installation instruction. Not free glut


Extract the downloaded ZIP file and make sure you find the following

    glut.h

    glut32.lib

    glut32.dll

If you have a 32 bits operating system, place glut32.dll to C:\Windows\System32\, if your operating system is 64 bits, place it to 'C:\Windows\SysWOW64\' (to your system directory)

Place glut.h C:\Program Files\Microsoft Visual Studio 12\VC\include\GL\ (NOTE: 12 here refers to your VS version it may be 8 or 10)

If you do not find VC and following directories.. go on create it.

Place glut32.lib to C:\Program Files\Microsoft Visual Studio 12\VC\lib\

Now, open visual Studio and

    Under Visual C++, select Empty Project(or your already existing project)
    Go to Project -> Properties. Select 'All Configuration' from Configuration dropdown menu on top left corner
    Select Linker -> Input
    Now right click on "Additional Dependence" found on Right panel and click Edit

now type

    opengl32.lib

    glu32.lib

    glut32.lib

(NOTE: Each .lib in new line)

That's it... You have successfully installed OpenGL.. Go on and run your program.


