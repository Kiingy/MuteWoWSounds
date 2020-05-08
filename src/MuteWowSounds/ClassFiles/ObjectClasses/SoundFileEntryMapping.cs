using TinyCsvParser.Mapping;

namespace MuteWowSounds.ClassFiles.ObjectClasses {
    class SoundFileEntryMapping : CsvMapping<SoundFileEntry> {

        public SoundFileEntryMapping()
            : base() {
            MapProperty(0, x => x.ID);
            MapProperty(1, x => x.SoundKitID);
            MapProperty(2, x => x.FileDataID);
        }
    }
}
