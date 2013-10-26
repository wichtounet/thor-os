extern "C"
void kernel_main(){
   unsigned char *vidmem = (unsigned char*) 0x0B8000;

   *vidmem++ = 'a';
   *vidmem++ = 0;
   *vidmem++ = 'b';
}
