# Programming-an-RTS
Programming an RTS by Carl Granberg. <br/>
https://archive.org/details/programming-an-rts-game-with-direct-3d<br/>
I bought this book many years ago and never got around to working through it/understanding it. <br/>
The original code is DirectX 9, which still runs for the most part. A few minor tweeks I think.<br/>
To build original code, you'll need the SDK here: https://www.microsoft.com/en-au/download/details.aspx?id=6812 <br/>
DXD9 branch will hold original code.<br/>
This is a GLFW (wsi)/Vulkan implementation.<br/>
You'll need the Vulkan SDK installed.<br/>
Build system is CMake (targeting Windows mostly, so you'll need to change it for another target), which I don't understand well, so if it compiles, yay!<br/>

Basic build using git/cmake from command line:<br/>
Clone with git...<br/>
```diff
git clone  repository-.... (click code button for repository link)
cd repository-folder
git submodules update
```
Build with cmake...
```diff
mkdir build --out of source build?
cd build
cmake .. --should run using current generator 
or
cmake -G "Visual Studio 16 2019" ..--select specific generator
Open in IDE to build or 
use --build e.g. cmake .. --build
```
As mentioned, not a cmake expert, so some messing about will be required for your system.



