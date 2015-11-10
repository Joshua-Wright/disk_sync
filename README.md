# disk_sync
Syncs disk images using a hash index to avoiding reading the backup image except when necessary. Uses multiple threads to calculate hashes as fast as possible, and intelligently marks empty blocks as sparse on output. 

#### Example Use Case:
Back up a SSD to an external HDD: The bottleneck will be the CPU using all available cores to calculate hashes of each block.  

#### Usage:
sync_images \<path_to_config_file\>

### Configuration File:
* [JSON](https://en.wikipedia.org/wiki/JSON) format
* fields marked with a * are required
* *```input```: input file path
* *```output```: output file path
* *```blocksize```: blocksize (integer)
* ```threads```: number of threads to use. Defaults to number of available cores
* ```status update```: whether to output progress information. Defaults to ```true```
* ```output interval```: how often to output progress in seconds. Defaults to 1
* ```sparse output```: whether to sparsify output file. Defaults to ```true```


output file must be on a filesystem that supports sparse files if use ```sparse output``` is enabled.  
If given blocksize is different than previous runs, behavior is undefined  

Credit to [GNU Coreutils](http://www.gnu.org/software/coreutils/coreutils.html) for the sha512 hash function, and to [Boost](http://www.boost.org/doc/libs/1_59_0/doc/html/property_tree/parsers.html#property_tree.parsers.json_parser) for the JSON parser