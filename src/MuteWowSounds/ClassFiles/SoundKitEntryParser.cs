using System;
using System.IO;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using TinyCsvParser;
using MuteWowSounds.ClassFiles.ObjectClasses;

namespace MuteWowSounds.ClassFiles {
    class SoundKitEntryParser {

        private readonly MuteSoundsUI parent;

        public SoundKitEntryParser(MuteSoundsUI parent) {
            this.parent = parent;
        }

        public List<SoundFileEntry> ParseCSV() {
            parent.LogMessage("Loading SoundKitEntry data", Color.Black, parent.Bold_Font);
            try {
                CsvParserOptions csvOp = new CsvParserOptions(false, ',');
                SoundFileEntryMapping csvMapper = new SoundFileEntryMapping();
                CsvParser<SoundFileEntry> csvParser = new CsvParser<SoundFileEntry>(csvOp, csvMapper);
                if (Directory.Exists(parent.Folder_SoundDataLocation) && File.Exists(parent.Folder_SoundDataLocation + "soundkitentry.csv")) {
                    var result = csvParser.ReadFromFile(parent.Folder_SoundDataLocation + "soundkitentry.csv", Encoding.ASCII).Where(x => x.IsValid).Select(x => x.Result).ToList();
                    parent.LogMessage(" ...done!\n", Color.Green, parent.Bold_Font);
                    return result;
                } else {
                    DataNotFound();
                    return null;
                }
            } catch (Exception ex) {
                parent.AppendError(ex.Message, 2);
                return null;
            }
        }

        private void DataNotFound() {
            parent.LogMessage(" ...File 'soundkitentry.csv' not found. Stopping.", parent.eColor, parent.Bold_Font);
        }
    }
}
