# I don't use Makefile, because it's really hard to use.
# use bash shell instead

if [ ! -e  /usr/local/include/uv.sh ]; then
git clone https://github.com/libuv/libuv.git && cd libuv && sh autogen.sh && ./configure && make && make install || echo "Install Libuv Fail" && exit
fi


gcc tun.c -o tun -luv
