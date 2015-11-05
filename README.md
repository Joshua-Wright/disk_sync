# disk_sync
syncs disk images using a hash index to avoiding reading the whole backup image

### Usage:
sync_images \<input file\> \<output file\> \<blocksize\>
* input file may be a device file or a regular file
* output file must be a regular file, as \<output file\>.hash is also created
* blocksize is required and must not chnage between usages

output file must be on a filesystem that supports sparse files

if given blocksize is different than previous runs, behavior is undefined

Credit to [GNU Coreutils](http://www.gnu.org/software/coreutils/coreutils.html) for the sha512 hash function
