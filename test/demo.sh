mkdir dir0
cd dir0
date > file11
echo file12 > file12
ln -s ./dir2/file21 link21
mkdir dir2
cd dir2
mkfifo fifo1
echo file21 >file21
echo file22 >file22
ln ../file11 hlink11
exit