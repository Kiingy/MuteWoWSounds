using MuteWowSounds.ClassFiles.ObjectClasses;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;

namespace MuteWowSounds.ClassFiles {
    class TargetSoundFileParser {


        private readonly MuteSoundsUI parent;

        public TargetSoundFileParser(MuteSoundsUI parent) {
            this.parent = parent;
        }

        public List<SoundList> ReadSoundTargetFiles() {
            List<SoundList> fileData = new List<SoundList>();

            // Get a list of all the text files in the target folder.
            // This gets the full path of files.
            string[] files = Directory.GetFiles(parent.Folder_SoundTargetFolder);
            string[] temp;

            // Get just the file name.
            for (int i = 0; i < files.Length; i++) {
                // Split each path at folder breaks. 
                temp = files[i].Split('\\');
                // Last split contains just the file name.
                files[i] = temp[temp.Length - 1];
                if (File.Exists(parent.Folder_SoundTargetFolder + files[i])) {
                    SoundList tempList = ParseFile(files[i]);
                    if (tempList != null) {
                        fileData.Add(tempList);
                    }
                } else {
                    parent.AppendError("Could not find file: " + files[i] + ".\n", 3);
                }
            }
            return fileData;
        }

        // Parse a given file.
        private SoundList ParseFile(string fileName) {
            parent.LogMessage("File: ", Color.Black, parent.Default_Font);
            parent.LogMessage(fileName + " ", Color.Black, parent.Default_Font);
            bool hasError = false;
            string errorLogs = "";
            int errorCount = 0;

            SoundList newList = new SoundList {
                // Names the list the same as the filename with file extension.
                ListName = fileName
            };

            using (StreamReader file = new StreamReader(parent.Folder_SoundTargetFolder + fileName)) {
                int lineNumber = 1;
                string lineData;
                string[] lineDataSplit;
                SoundTarget soundTarget;

                while ((lineData = file.ReadLine()) != null) {
                    // Ignore comment lines.
                    if (!lineData.Trim().StartsWith("#")) {
                        if (!String.IsNullOrEmpty(lineData.Trim())) {
                            try {
                                // First we split the comment and soundkit data.
                                lineDataSplit = lineData.Split('-');
                                // This will throw if there isn't exactly one -
                                if (!(lineDataSplit.Length == 2)) {
                                    throw new InvalidFormatException("Must be exactly one '-' character per line.");
                                }
                                soundTarget = new SoundTarget {
                                    // Set the line comment.
                                    EntryComment = lineDataSplit[0].Trim()
                                };
                                // Second we split the soundkit data IDs and add them one at a time.
                                lineDataSplit = lineDataSplit[1].Split(',');
                                for (int i = 0; i < lineDataSplit.Length; i++) {
                                    // Int parsing error.
                                    try {
                                        if (lineDataSplit[i].StartsWith("s")) {
                                            // For sounds not part of SoundKitID.
                                            soundTarget.SingleSounds.Add(ulong.Parse(lineDataSplit[i].Replace("s", "")));
                                        } else {
                                            soundTarget.SoundKitIDs.Add(ulong.Parse(lineDataSplit[i]));
                                        }
                                    } catch {
                                        hasError = true;
                                        errorLogs = errorLogs + "Line: '" + lineNumber + "' Input Given: '" + lineDataSplit[i] + "' Error: Invalid SoundID or SoundKitID. (Must be an integer)\n";
                                        errorCount++;
                                    }
                                }
                                newList.targets.Add(soundTarget);
                            } catch (Exception ex) {
                                hasError = true;
                                errorLogs = errorLogs + "Line: '" + lineNumber + "' Error: Incorrect format. (" + ex.Message + ")\n";
                                errorCount++;
                            }
                        }
                    }
                    lineNumber++;
                }
            }

            if (hasError) {
                parent.AppendError("File: " + fileName + "\n" + errorLogs + "\n", 0);
                parent.LogMessage(" ...done with (" + errorCount + ") error(s).\n", Color.Orange, parent.Bold_Font);
            } else {
                parent.LogMessage(" ...done!\n", Color.Green, parent.Bold_Font);
            }

            // If its empty, skip it.
            if (newList.targets.Count == 0) {
                // If it was empty but not from a result of an error.
                if (!hasError) {
                    parent.LogMessage(" ...file is empty. Skipping.\n", Color.Orange, parent.Bold_Font);
                }
                return null;
            } else {
                return newList;
            }
        }
    }
}
