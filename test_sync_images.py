#!/usr/bin/env python
# (c) Copyright 2015 Josh Wright
import os
import sys
import time
import filecmp
import random

do_random_changes = True
# do_random_changes = False

BYTES_PER_GB = 1024*1024*1024
BYTES_PER_MB = 1024*1024
TEST_FILE_SIZE = BYTES_PER_GB

test_dir = "/home/j0sh/Documents/disk_sync_test"
test_cfg_path = "/home/j0sh/Dropbox/code/Cpp/disk_sync/test_cfg.json"
sync_binary_path = "/home/j0sh/Dropbox/code/Cpp/disk_sync/sync_images"
# sync_binary_path = "/sync_images"
test_1GB_basic = os.path.join(test_dir, "test_1GB_basic.img")
test_1GB_sequential = os.path.join(test_dir, "test_1GB_sequential.img")
blocksize = 4096




def make_one_gb_file(path, fill='basic'):
	with open(path, 'w') as f:
		if fill == "basic":
			for x in range(10):
				f.write("#" * int(TEST_FILE_SIZE/10) + '\n')
		elif fill == "sequential":
			chars = ''.join(chr(x) for x in range(128))
			for x in range(int(TEST_FILE_SIZE/128)):
				f.write(chars)

def make_sparse_file(path,size):
	print("Making sparse file: " + path)
	os.system("truncate -s " + str(int(size)) + " " + path)

def make_random_changes(path, count=500):
	print("Making random changes to file: " + path)
	size = os.path.getsize(path)
	# chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890()!@#$%^&*()_+[]{}|;:,./<>?`~"
	# with open(path, 'w+b') as f:
	# with open(path, 'a+b') as f:
	with open(path, 'r+b') as f:
		for x in range(count):
			f.seek(random.randint(0, size))
			f.write(os.urandom(random.randint(0, 2*blocksize)))

def write_cfg_file(cfg_path, test_file_path, blocksize):
	with open(cfg_path, 'w') as output_fobj:
		# output_fobj.write(
		output_fobj.write('{'                                            )
		output_fobj.write('	"input": "'+test_file_path+'",'              )
		output_fobj.write('	"output": "'+(test_file_path+".synced")+'",' )
		output_fobj.write('	"blocksize": '+str(blocksize)+','            )
		output_fobj.write('	"threads": 4,'                               )
		output_fobj.write('	"output interval": 1,'                       )
		output_fobj.write('	"sparse output": true,'                      )
		output_fobj.write('	"status update": true'                       )
		output_fobj.write('}'                                            )

if not os.path.exists(test_1GB_basic):
	make_one_gb_file(test_1GB_basic)
if not os.path.exists(test_1GB_sequential):
	make_one_gb_file(test_1GB_sequential, fill="sequential")


# for file_path in [test_1GB_basic]:
for file_path in [test_1GB_basic, test_1GB_sequential]:
	if do_random_changes:
		make_random_changes(file_path)
	if not os.path.exists(file_path + ".synced.hash"):
		make_sparse_file(file_path + ".synced.hash",  TEST_FILE_SIZE/64) # 64 is size of sha512 hash
	if not os.path.exists(file_path + ".synced"):
		make_sparse_file(file_path + ".synced",  TEST_FILE_SIZE)
	# cmd_string = " ".join([sync_binary_path, file_path, file_path+".synced", str(blocksize)])
	write_cfg_file(test_cfg_path, file_path, blocksize)
	cmd_string = sync_binary_path + " " + test_cfg_path
	print("Running: " + cmd_string)
	t0 = time.time()
	os.system(cmd_string)
	t1 = time.time()
	print("Time: " + str(t1 - t0))
	if not filecmp.cmp(file_path, file_path+".synced", shallow=False):
		print("TEST FAILED: Files are not equal: " + file_path)
