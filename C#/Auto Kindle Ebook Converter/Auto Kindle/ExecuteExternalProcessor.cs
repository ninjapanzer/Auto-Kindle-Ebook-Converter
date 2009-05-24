using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Auto_Kindle
{
    class ExecuteExternalProcessor
    {
        ProcessInformation appInfo;
        //private string applicationString;
        //private string applicationArgString;
        //private string applicationStdOutput;

        ~ExecuteExternalProcessor()
        {

        }

        public ExecuteExternalProcessor()
        {
            this.appInfo.applicationString = null;
            this.appInfo.applicationArgString = null;
            this.appInfo.applicationStdOutput = null;
            this.appInfo.applicationErrorOutput = null;
            this.appInfo.applicationWholeError = null;
            //this.applicationString = null;
            //this.applicationArgString = null;
            //this.applicationStdOutput = null;
        }

        public ExecuteExternalProcessor(ProcessInformation appInfo)
        {
            this.appInfo = appInfo;
            ExecuteProcess();
        }

        public ExecuteExternalProcessor(string applicationString, string applicationArgString)
        {
            this.appInfo.applicationString = applicationString;
            this.appInfo.applicationArgString = applicationArgString;
            this.appInfo.applicationStdOutput = null;
            ExecuteProcess();
        }

        public ProcessInformation GetappInfo() { return this.appInfo; }
        public string GetapplicationString() { return this.appInfo.applicationString; }
        public string GetapplicationArgString() { return this.appInfo.applicationArgString; }
        public string GetapplicationStdOutput() { return this.appInfo.applicationStdOutput; }
        public string GetapplicationErrorOutput() { return this.appInfo.applicationErrorOutput; }
        public string GetapplicationWholeError() { return this.appInfo.applicationWholeError; }

        public void SetappInfo(ProcessInformation appInfo) { this.appInfo = appInfo; }
        public void SetapplicationString(string applicationString) { this.appInfo.applicationString = applicationString; }
        public void SetapplicationArgString(string applicationArgString) { this.appInfo.applicationArgString = applicationArgString; }
        //public void SetapplicationStdOutput(string applicationStdOutput) { this.appInfo.applicationStdOutput = applicationStdOutput; }

        public bool ExecuteProcess()
        {
            if (appInfo.applicationArgString != null)
            {
                Process runproc = new Process();
                runproc.StartInfo.CreateNoWindow = true;
                runproc.StartInfo.Arguments = this.appInfo.applicationArgString;
                if (appInfo.applicationSwitch != null)
                {
                    runproc.StartInfo.Arguments = "\"" + this.appInfo.applicationString + "\"" + " " + this.appInfo.applicationSwitch;
                }
                runproc.StartInfo.UseShellExecute = false;
                runproc.StartInfo.FileName = this.appInfo.applicationString;
                runproc.StartInfo.RedirectStandardOutput = true;
                runproc.StartInfo.RedirectStandardInput = true;
                runproc.StartInfo.RedirectStandardError = true;
                runproc.Start();
                //runproc.StandardInput.WriteLine("input Trap");
                runproc.WaitForExit();
                this.appInfo.applicationStdOutput = runproc.StandardOutput.ReadToEnd();
                this.appInfo.applicationErrorOutput = runproc.StandardError.ReadToEnd();
                this.appInfo.applicationWholeError = this.appInfo.applicationStdOutput + "\n" + this.appInfo.applicationErrorOutput;
                return true;
            }
            return false;
        }
    }
}
