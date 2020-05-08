using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using MuteWowSounds.ClassFiles;
using MuteWowSounds.ClassFiles.ObjectClasses;
using System.Threading;
using System.IO;

namespace MuteWowSounds {
    public partial class MuteSoundsUI : Form {

        public string VersionDate = "15/05/20";
        public string VersionNum = "1.0";
        private Thread WorkThread;

        private readonly SoundKitEntryParser CSVParser;
        private readonly TargetSoundFileParser TSFParser;
        private readonly LuaWriter LWriter;
        private readonly ErrorCollector ECollector;


        public readonly string Folder_Output;
        public readonly string Folder_SoundDataLocation;
        public readonly string Folder_SoundTargetFolder;

        private bool pError = false;
        private bool startError = false;

        public readonly Font Default_Font;
        public readonly Font Bold_Font;
        public Color eColor = Color.FromArgb(179, 0, 0);


        private List<SoundFileEntry> SoundData;


        public MuteSoundsUI() {
            InitializeComponent();
            this.CSVParser = new SoundKitEntryParser(this);
            this.TSFParser = new TargetSoundFileParser(this);
            this.LWriter = new LuaWriter(this);
            this.ECollector = new ErrorCollector(this);
            this.Default_Font = TextBoxLog.Font;
            this.Bold_Font = new Font(Default_Font, FontStyle.Bold);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.FormBorderStyle = FormBorderStyle.FixedSingle;
            versionLabel.Text = "v" + VersionNum + " - " + VersionDate;

            this.Folder_Output = Directory.GetCurrentDirectory() + "\\Output\\";
            this.Folder_SoundDataLocation = Directory.GetCurrentDirectory() + "\\SoundKitData\\";
            this.Folder_SoundTargetFolder = Directory.GetCurrentDirectory() + "\\SoundTargets\\";
            if (!Directory.Exists(Folder_Output)) {
                Directory.CreateDirectory(Folder_Output);
            }

            if(!Directory.Exists(Folder_SoundDataLocation) || !File.Exists(Folder_SoundDataLocation+"soundkitentry.csv")) {
                startError = true;
                LogMessage("Couldn't find 'soundkitentry.csv' at location: \n'"+Folder_SoundDataLocation +"'\n", eColor, Bold_Font);
                RunButton.Enabled = false;
            } 
            
            if (!Directory.Exists(Folder_SoundTargetFolder)) {
                startError = true;
                LogMessage("Couldn't find directory: \n'" + Folder_SoundTargetFolder + "'\n", eColor, Bold_Font);
                RunButton.Enabled = false;
            }
            
            if (!startError) {
                LogMessage("Ready!", Color.Black, Bold_Font);
            }
        }


        public void LogMessage(string message, Color color, Font font) {
            if (this.TextBoxLog.InvokeRequired) {
                LogMessageCallback CB = new LogMessageCallback(LogMessage);
                this.Invoke(CB, new object[] { message, color, font });
            } else {
                TextBoxLog.AppendText(message, color, font);
                TextBoxLog.ScrollToCaret();
            }
        }
        delegate void LogMessageCallback(string message, Color color, Font font);

        private void RunButton_Click(object sender, EventArgs e) {
            if (!startError) {
                if (WorkThread == null || !WorkThread.IsAlive) {
                    LogMessage("\nStarting table generation.\n\n", Color.Green, Bold_Font);
                    WorkThread = new Thread(() => RunGenerator());
                    WorkThread.Start();
                } else {
                    RunError.Text = "Program is already running. Please wait.";
                    pError = true;
                }
            } else {
                LogMessage("\nProgram has been disabled.\n", eColor, Default_Font);
            }
        }

        private void RunGenerator() {
                LogMessage("Parsing sound target files.\n", Color.Black, Bold_Font);
                List<SoundList> TargetData = TSFParser.ReadSoundTargetFiles();
                LogMessage("\n", Color.White, Default_Font);
            if (TargetData.Count > 0) {
                if (this.SoundData == null) {
                    this.SoundData = CSVParser.ParseCSV();
                }
                if (this.SoundData != null) {
                    LWriter.WriteLuaFile(LWriter.CreateLuaTableString(TargetData, this.SoundData), "CustomSounds.lua");
                    ECollector.WriteErrorLog(this.Folder_Output, "ErrorLog.txt");
                    LogMessage("\nComplete!\n", Color.Green, Bold_Font);
                    if (pError) {
                        RunLabelText("");
                    }
                }
            } else {
                LogMessage("No files to parse! Stopping.\n", eColor, Bold_Font);
            }
        }

        private void RunLabelText(string message) {
            if (this.RunError.InvokeRequired) {
                this.RunError.BeginInvoke((MethodInvoker)delegate () { this.RunError.Text = message.ToString(); });
            } else {
                RunError.Text = message;
            }
        }

        private void ExitBtn_Click(object sender, EventArgs e) {
            Application.Exit();
        }

        public void AppendError(string errorMsg, int type) {
            this.ECollector.AppendError(errorMsg, type);
        }

        public string ReplaceLastOccurrence(string Source, string Find, string Replace) {
            if (Source == null) return Source;
            int place = Source.LastIndexOf(Find);

            if (place == -1)
                return Source;

            string result = Source.Remove(place, Find.Length).Insert(place, Replace);
            return result;
        }

        private void ClearLog_Click(object sender, EventArgs e) {
            TextBoxLog.Clear();
        }
    }

    public static class RichTextBoxExtensions {
        public static void AppendText(this RichTextBox box, string text, Color color, Font font) {
            box.SelectionStart = box.TextLength;
            box.SelectionLength = 0;
            box.SelectionColor = color;
            if (font != null) {
                box.SelectionFont = font;
            }
            box.AppendText(text);
            box.SelectionColor = box.ForeColor;
        }
    }
}
