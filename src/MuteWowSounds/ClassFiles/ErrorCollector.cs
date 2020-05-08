using System.IO;

namespace MuteWowSounds.ClassFiles {
    class ErrorCollector {
        private string printString;
        private readonly MuteSoundsUI parent;
        private bool printLog = false;
        private bool parseError = false;
        private bool matchError = false;
        private bool soundSourceError = false;
        private bool soundTargetError = false;

        public ErrorCollector(MuteSoundsUI parent) {
            this.parent = parent;
            printString = "";
        }

        public void AppendError(string error, int type) {
            if (type == 0 && !parseError) {
                printString += "File parsing errors:\n------------------------\n";
                parseError = true;
            } else if (type == 1 && !matchError) {
                printString += "FileDataID matching errors:\n------------------------\n";
                matchError = true;
            } else if (type == 2 && !soundSourceError) {
                printString += "SoundKitData parse error:\n------------------------\n";
                soundSourceError = true;
            } else if (type == 3 && !soundTargetError) {
                printString += "Sound target parse error:\n------------------------\n";
                soundTargetError = true;
            }
            printString += error;
            printLog = true;
        }

        public void WriteErrorLog(string fileDir, string fileName) {
            if (printLog) {
                parent.LogMessage("\nWriting error log >> Output\\ErrorLog.txt.\n", parent.eColor, parent.Bold_Font);
                StreamWriter sw = new StreamWriter(fileDir + fileName, false);
                sw.WriteLine(printString);
                sw.Close();
                ClearErrors();
            }
        }

        private void ClearErrors() {
            printLog = false;
            parseError = false;
            matchError = false;
            soundSourceError = false;
            printString = "";
        }

    }
}
