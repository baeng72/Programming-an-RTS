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
<p>
git clone <i>repository-....</i> (click click button for this)<br/>
cd <i>repository-folder</i><br/>
git submodules update
</p>
<p>
mkdir build --out of source build?<br/>
cd build<br/>
cmake .. --should run using current generator <br/>
or<br/>
cmake -G "Visual Studio 16 2019" ..--select specific generator<br/>
Open in IDE to build or <br/>
use --build e.g. <i>cmake .. --build</i>
</p>
As mentioned, not a cmake expert, so some messing about will be required for your system.



