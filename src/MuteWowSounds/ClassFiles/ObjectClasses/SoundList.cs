using System.Collections.Generic;


namespace MuteWowSounds.ClassFiles.ObjectClasses {
    class SoundList {
        public SoundList() {
            targets = new List<SoundTarget>();
        }
        public string ListName { get; set; }
        public List<SoundTarget> targets { get; set; }

        public override string ToString() {
            if (targets.Count > 0) {
                int count = 1;
                string str = "Name: " + ListName;

                foreach (SoundTarget t in targets) {
                    str = str + "\n" + count.ToString() + ") Comment: " + t.EntryComment + ", Values: ";
                    foreach (int i in t.SoundKitIDs) {
                        str = str + t.SoundKitIDs[i] + ",";

                    }
                    count++;
                }
                return str;
            } else {
                return "";
            }
        }
    }
}
