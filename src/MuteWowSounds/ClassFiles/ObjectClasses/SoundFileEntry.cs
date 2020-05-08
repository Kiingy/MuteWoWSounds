using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MuteWowSounds.ClassFiles.ObjectClasses {
    class SoundFileEntry {
        public int ID { get; set; }
        public ulong SoundKitID { get; set; }
        public ulong FileDataID { get; set; }

        public override string ToString() {
            return FileDataID.ToString();
        }
    }
}
