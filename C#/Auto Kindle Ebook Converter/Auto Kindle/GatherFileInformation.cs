using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Auto_Kindle
{
    class GatherFileInformation : ExecuteExternalProcessor
    {

        //private string fileInString;
        //private string fileOutString;
        private string fileName;
        private string fileLocation;
        private string fileExtension;
        public fileLocation files;

        public GatherFileInformation()
        {
            this.files.fileIn = null;
            this.files.fileOut = null;
            this.fileName = null;
            this.fileLocation = null;
            this.fileExtension = null;
        }

        public GatherFileInformation(string fileInString, string fileOutString)
        {
            this.files.fileIn = fileInString;
            this.files.fileOut = fileOutString;
            this.fileName = null;
            this.fileLocation = null;
            this.fileExtension = null;
        }
        public GatherFileInformation(string fileInString)
        {
            this.files.fileIn = fileInString;
            this.files.fileOut = null;
            this.fileName = null;
            this.fileLocation = null;
            this.fileExtension = null;
            ExecuteSetInternals();

        }

        //public string GetfileInString() { return this.files.fileIn; }
        //public string GetfileOutString() { return this.files.fileOut; }
        public string GetfileName() { return this.fileName; }
        public string GetfileLocation() { return this.fileLocation; }
        public string GetfileExtension() { return this.fileExtension; }
        public fileLocation Getfiles() { return this.files; }

        //public void SetfileInString(string fileInString) { this.files.fileIn = fileInString; }
        //public void SetfileOutString(string fileOutString) { this.files.fileOut = fileOutString; }
        protected void Setfiles(fileLocation files) { this.files = files; }
        public void SetfileName(string fileName) { this.fileName = fileName; }
        public void SetfileLocation(string fileLocation) { this.fileLocation = fileLocation; }
        public void SetfileExtension(string fileExtension) { this.fileExtension = fileExtension; }

        public void ExecuteSetInternals()
        {
            this.ExecuteExtractFilename();
            this.ExecuteEscapify();
        }
        // Creats a string with quote marks around it from the normal strings to be passed as a literal
        private void ExecuteEscapify()
        {
            this.files.fileIn_Escape = "\"" + this.files.fileIn + "\"";
            this.files.fileLocationDir_Escape = "\"" + this.files.fileLocationDir + "\"";
            this.files.fielName_Escape = "\"" + this.files.fileName + "\"";
            this.files.fileOut_Escape = "\"" + this.files.fileOut + "\"";
        }

        public string ExecuteExtractFilename()
        {
            string localfileInString = this.files.fileIn;
            string localfileName = null, localfileLocation = null;
            char[] splitArray;
            var fileNameQueue = new Queue<string>();
            int splitArrayLength, fileNamePosition = 0;
            if (this.files.fileIn == null)
            {
                MessageBox.Show("fileInString Null BAD BOY");
            }
            else
            {
                splitArray = localfileInString.ToCharArray();
                splitArrayLength = splitArray.Length - 1;
                for (int i = splitArrayLength; i > 0; i--)
                {
                    if (splitArray[i] == '\\')
                    {
                        fileNamePosition = i + 1;
                        break;
                    }
                }
                //Extracts Filename at poition 1 + the location of the last \ slash
                fileNameQueue.Clear();
                for (int i = fileNamePosition; i <= splitArrayLength; i++)
                {
                    fileNameQueue.Enqueue(splitArray[i].ToString());
                }
                foreach (string element in fileNameQueue)
                {
                    localfileName = localfileName + element;
                }
                //Extracts File Location at position - 1 the location of the last slash
                //so we start 0 to the fileNamePosition - 2 which ends on the last char before the last slash
                fileNameQueue.Clear();
                for (int i = 0; i <= fileNamePosition - 2; i++)
                {
                    fileNameQueue.Enqueue(splitArray[i].ToString());
                }
                foreach (string element in fileNameQueue)
                {
                    localfileLocation = localfileLocation + element;
                }
                fileNameQueue.Clear();
                this.files.fileLocationDir = localfileLocation;
                //this.fileName = localfileName;
                this.files.fileName = this.ExecuteProcessExt(localfileName);
                //this.ExecuteProcessExt();
            }
            return localfileName;
        }


        //can be passed externally as long as a filename without a path is passed
        // to the function. The function returns the filename and sets the file ext obj
        // this will always overwrite the file ext of the input file when run
        //This is also internally executed for return byt the ExecuteExtractFilename() function
        // to finish generation of the filename without the ext attached 
        public string ExecuteProcessExt(string fileNameString)
        {
            if (this.files.fileExt == null)
            {
                string localfileName = fileNameString, fileName_NoExt = null, localfileExt = null;
                char[] splitArray;
                var fileNameQueue = new Queue<string>();
                int splitArrayLength, fileNamePosition = 0;
                splitArray = localfileName.ToCharArray();
                splitArrayLength = splitArray.Length - 1;
                for (int i = splitArrayLength; i > 0; i--)
                {
                    if (splitArray[i] == '.')
                    {
                        fileNamePosition = i + 1;
                        break;
                    }
                }
                fileNameQueue.Clear();
                for (int i = fileNamePosition; i <= splitArrayLength; i++)
                {
                    fileNameQueue.Enqueue(splitArray[i].ToString());
                }
                foreach (string element in fileNameQueue)
                {
                    localfileExt = localfileExt + element;
                }
                fileNameQueue.Clear();
                for (int i = 0; i <= fileNamePosition - 2; i++)
                {
                    fileNameQueue.Enqueue(splitArray[i].ToString());
                }
                foreach (string element in fileNameQueue)
                {
                    fileName_NoExt = fileName_NoExt + element;
                }
                fileNameQueue.Clear();
                this.files.fileExt = localfileExt;
                return fileName_NoExt;

            }
            return null;
        }

        public string ExecuteFileOpen()
        {
            OpenFileDialog fileDialog = new OpenFileDialog();
            fileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments).ToString();
            fileDialog.Filter = "All Files (*.*)|*.*";
            fileDialog.FilterIndex = 1;
            fileDialog.RestoreDirectory = true;
            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                if (this.files.fileIn == null)
                {
                    this.files.fileIn = fileDialog.FileName;
                }
                this.ExecuteSetInternals();
                return fileDialog.FileName;
            }
            return null;
        }

        public string ExecuteFolderSelect()
        {
            FolderBrowserDialog folderDialog = new FolderBrowserDialog();
            if (folderDialog.ShowDialog() == DialogResult.OK)
            {
                if (this.files.fileOut == null)
                {
                    this.files.fileOut = folderDialog.SelectedPath;
                }
                return folderDialog.SelectedPath;
            }
            return null;
        }

    }
}
