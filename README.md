# disk_sync
Syncs disk images using a hash index to avoiding reading the backup image except when necessary. Uses multiple threads to calculate hashes as fast as possible, and intelligently marks empty blocks as sparse on output. 

#### Example Use Case:
Back up a SSD to an external HDD: The bottleneck will be the CPU using all available cores to calculate hashes of each block.  

#### Usage:
sync_images \<input file\> \<output file\> \<blocksize\>
* input file may be a device file or a regular file
* output file must be a regular file, as \<output file\>.hash is also created
* blocksize is required and must not chnage between usages

output file must be on a filesystem that supports sparse files

if given blocksize is different than previous runs, behavior is undefined

Credit to [GNU Coreutils](http://www.gnu.org/software/coreutils/coreutils.html) for the sha512 hash function
