using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Auto_Kindle
{
    class ArgumentHandler : GatherFileInformation
    {
        private int argLen;
        //private int currentLocation;
        private string[] argArray;
        private Queue<string> fileLocations;
        private Queue<string> optQueue;
        public fileLocation files;


        public ArgumentHandler(string[] args)
        {
            this.fileLocations = new Queue<string>();
            this.optQueue = new Queue<string>();
            this.argArray = args;
            this.argLen = args.Length;
            files.fileIn = null;
            files.fileOut = null;
            SortArguments();
        }

        public ArgumentHandler()
        {
            this.fileLocations = new Queue<string>();
            this.optQueue = new Queue<string>();
            argArray = null;
            this.argLen = 0;
            //this.currentLocation=0;
        }

        public void Clear()
        {
            this.fileLocations.Clear();
            this.optQueue.Clear();
        }

        public Queue<string> GetoptQueue() { return this.optQueue; }

        public static bool ArgumentOption(string element)
        {
            char[] argTest;
            argTest = element.ToCharArray();
            if (argTest[0] == '-')
            {
                return true;
            }
            return false;
        }

        public static bool ArgumentLocation(string element)
        {
            char[] argTest;
            argTest = element.ToCharArray();
            if (argTest[0] != '-')
            {
                return true;
            }
            return false;
        }

        public void SortArguments()
        {
            int fileCount = 0;
            //int optionCount = 0;
            if (this.argLen > 0)
            {
                foreach (string element in argArray)
                {
                    if (ArgumentOption(element))
                    {
                        this.optQueue.Enqueue(element.ToString());
                        //MessageBox.Show(element.ToString());
                        //optionCount++;
                    }
                    else
                    {
                        this.fileLocations.Enqueue(element);
                        //MessageBox.Show(element.ToString());
                        fileCount++;
                    }
                }
                if (fileCount == 1)
                {
                    this.files.fileIn = this.fileLocations.Dequeue();
                    this.fileLocations.Clear();
                }
                else if (fileCount > 1)
                {
                    this.files.fileIn = this.fileLocations.Dequeue();
                    this.files.fileOut = this.fileLocations.Dequeue();
                    this.fileLocations.Clear();
                }
                this.Setfiles(files);
                this.ExecuteExtractFilename();
                this.files = this.Getfiles();
            }

        }
    }
}
