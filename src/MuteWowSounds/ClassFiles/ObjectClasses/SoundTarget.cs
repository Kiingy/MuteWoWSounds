using System.Collections.Generic;

namespace MuteWowSounds.ClassFiles {
    class SoundTarget {

        public string EntryComment { get; set; }
        public List<ulong> SoundKitIDs { get; set; }
        public List<ulong> SingleSounds { get; set; }

        public SoundTarget() {
            this.SoundKitIDs = new List<ulong>();
            this.SingleSounds = new List<ulong>();
        }
    }
}
