using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Auto_Kindle
{
    public struct fileLocation
    {
        public string fileIn;
        public string fileIn_Escape;
        public string fileOut;
        public string fileOut_Escape;
        public string fileLocationDir;
        public string fileLocationDir_Escape;
        public string fileName;
        public string fielName_Escape;
        public string fileExt;
        public string fileTemp;
    };

    public struct ProcessInformation
    {
        public string applicationString;
        public string applicationArgString;
        public string applicationStdOutput;
        public string applicationErrorOutput;
        public string applicationWholeError;
        public string applicationSwitch;
    };
}
