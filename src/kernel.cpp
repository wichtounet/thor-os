extern "C"
void kernel_main(){
   unsigned char *vidmem = (unsigned char*) 0xB80000;

   *vidmem = 'a';
}
