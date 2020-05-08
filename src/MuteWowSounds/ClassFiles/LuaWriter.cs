using MuteWowSounds.ClassFiles.ObjectClasses;
using System.Collections.Generic;
using System.Drawing;
using System.IO;

namespace MuteWowSounds.ClassFiles {
    class LuaWriter {
        private readonly MuteSoundsUI parent;

        public LuaWriter(MuteSoundsUI parent) {
            
            this.parent = parent;
        }


        public void WriteLuaFile(string text, string name) {
            StreamWriter sw = new StreamWriter(parent.Folder_Output + name);
            sw.WriteLine(text);
            sw.Close();
        }

        public string CreateLuaTableString(List<SoundList> SoundTargets, List<SoundFileEntry> SoundKitData) {
            string tableString = "-- Created with MAWoWSoundsGenerator v" + parent.VersionNum + ", vDate " + parent.VersionDate + ".\n" +
                "-- Creation Date: \n" +
                "MAWowSoundsCustom = {\n";
            parent.LogMessage("\nMatching FileDataIDs:\n", Color.Black, parent.Bold_Font);
            foreach (SoundList fileList in SoundTargets) {
                parent.LogMessage("File: " + fileList.ListName, Color.Black, parent.Default_Font);
                tableString = tableString + "   -- File: " + fileList.ListName + "\n" + GetFileDataID(fileList.ListName, SoundKitData, fileList);

            }
            tableString += "}";
            return tableString.Remove(tableString.LastIndexOf(","), 1);
        }


        private string GetFileDataID(string ListName, List<SoundFileEntry> SoundKitData, SoundList SoundKitIds) {
            List<SoundFileEntry> FilteredSoundData;
            string formattedString = "";
            string soundKitErrors = "";
            int errorCount = 0;
            bool hasError = false;

            // For each SoundKitID we are targetting for a mute.
            foreach (SoundTarget st in SoundKitIds.targets) {
                // Append the entry comment to the top of the current block.
                formattedString = formattedString + "   -- " + st.EntryComment + "\n";
                foreach (ulong i in st.SoundKitIDs) {
                    // Create a filtered list that contains the FileDataIds with the matching SoundKitID
                    FilteredSoundData = SoundKitData.FindAll(x => x.SoundKitID == i);
                    if (FilteredSoundData.Count > 0) {
                        // Lua file indentation.
                        formattedString += "   ";
                        // Write each FileDataID in sequence.
                        foreach (SoundFileEntry SFE in FilteredSoundData) {
                            formattedString = formattedString + SFE.FileDataID + ",";
                        }
                        // Append the SoundKitID at the end as comment.
                        formattedString = formattedString + " -- " + i + "\n";
                    } else {
                        hasError = true;
                        soundKitErrors += "'"+i + "', ";
                        errorCount++;
                    }
                }

                // For sounds not part of a soundkit
                // In this case the sound ID is the FileDataID, no matching required.
                if (st.SingleSounds.Count > 0) {
                    foreach (int j in st.SingleSounds) {
                        formattedString = formattedString + j + ",";
                    }
                    formattedString += "-- Non soundkit sounds\n";
                }
            }
            if (hasError) {
                soundKitErrors = parent.ReplaceLastOccurrence(soundKitErrors, ", ", ".");
                parent.AppendError("File: " + ListName + ".txt\nNo matches found for: " + soundKitErrors + "\n\n", 1);
                parent.LogMessage(" ...done with (" + errorCount + ") error(s).\n", Color.Orange, parent.Bold_Font);
            } else {
                parent.LogMessage(" ...done!\n", Color.Green, parent.Bold_Font);
            }
            return formattedString;
        }
    }
}
