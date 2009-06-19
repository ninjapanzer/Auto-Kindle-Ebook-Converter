using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Auto_Kindle
{
    class GatherFileInformation : ExecuteExternalProcessor
    {
        private string _FileInPath;
        private string _TempPath;
        private string _FileOutPath;
        
        #region Public Properties
        public string FileInName
        {
            get
            {
                return Path.GetFileName(this._FileInPath);
            }
        }
        public string FileInLocation 
        {
            get
            {
                return Path.GetDirectoryName(this._FileInPath);
            }
        }
        public string FileInExtension 
        {
            get
            {
                return Path.GetExtension(this._FileInPath);
            }
        }
        public string FileInNameNoExtension
        {
            get
            {
                return Path.GetFileNameWithoutExtension(this._FileInPath);
            }
        }
        public string FileInFullPath
        {
            get
            {
                return this._FileInPath;
            }
            set
            {
                this._FileInPath = value;
            }
        }
        public string FileInEscaped{get {return "\"" + this._FileInPath + "\"";}}

        public string TempName
        {
            get
            {
                return Path.GetFileName(this._TempPath);
            }
        }
        public string TempLocation
        {
            get
            {
                return this._TempPath;
            }
        }
        public string TempExtension
        {
            get
            {
                return Path.GetExtension(this._TempPath);
            }
        }
        public string TempNoExtension
        {
            get
            {
                return Path.GetFileNameWithoutExtension(this._TempPath);
            }
        }
        public string TempEscaped { get { return "\"" + this._TempPath + "\""; } }

        public string FileOutName
        {
            get
            {
                return Path.GetFileName(this._FileOutPath);
            }
        }
        public string FileOutLocation
        {
            get
            {
                return this._FileOutPath;
            }
            set
            {
                this._FileOutPath = value;
            }
        }
        public string FileOutExtension
        {
            get
            {
                return Path.GetExtension(this._FileOutPath);
            }
        }
        public string FileOutNameNoExtension
        {
            get
            {
                return Path.GetFileNameWithoutExtension(this._FileOutPath);
            }
        }
        public string FileOutEscaped{get {return "\"" + this._FileOutPath + "\"";}}
        #endregion
        #region Constructors
        public GatherFileInformation(): this(null, null)
        {
        }
        public GatherFileInformation(string fileInString):this(fileInString, null)
        {
        }
        public GatherFileInformation(string fileInString, string fileOutString)
        {
            this._FileInPath= fileInString;
            this._TempPath = Properties.Settings.Default.TempPath + "\\" + this.FileInNameNoExtension;
            if (!Directory.Exists(this._TempPath))
            {
                Directory.CreateDirectory(this._TempPath);
            }
            this._FileOutPath = fileOutString;
        }
        
        #endregion

        public bool ExecuteFileOpen()
        {
            OpenFileDialog fileDialog = new OpenFileDialog();
            fileDialog.InitialDirectory = Properties.Settings.Default.EBookPath;
            fileDialog.Filter = "All Files (*.*)|*.*";
            fileDialog.FilterIndex = 1;
            fileDialog.RestoreDirectory = true;
            if (fileDialog.ShowDialog() == DialogResult.OK)
            {
                if (this._FileInPath == null)
                {
                    this._FileInPath = fileDialog.FileName;
                }
                Properties.Settings.Default.EBookPath = Path.GetDirectoryName(fileDialog.FileName);
                return true;
            }
            return false;
        }
        public bool ExecuteFolderSelect()
        {
            FolderBrowserDialog folderDialog = new FolderBrowserDialog();
            folderDialog.SelectedPath = Properties.Settings.Default.DefaultOutPath;
            if (folderDialog.ShowDialog() == DialogResult.OK)
            {
                if (this._FileOutPath == null)
                {
                    this._FileOutPath = folderDialog.SelectedPath;
                    this._TempPath = Properties.Settings.Default.TempPath + "\\" + this.FileInNameNoExtension;
                    if (!Directory.Exists(this._TempPath))
                    {
                        Directory.CreateDirectory(this._TempPath);
                    }
                }
                return true;
            }
            return false;
        }
    }
}
