int main() {
    asm volatile("int $0x80");
    asm volatile("int $0x80");
    asm volatile("int $0x80");
    while(1) { 
        // asm volatile("int $0x80");
    }
    return 0;
}
