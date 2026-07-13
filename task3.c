
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_USERS 10
#define MAX_FILES 50
#define KEY 0x5A

typedef struct { char user[32]; unsigned int hash; int role, active; } User;
typedef struct { char name[64], owner[32]; unsigned short perms; int enc, active; } FileMeta;

User users[MAX_USERS];
FileMeta files[MAX_FILES];
char current_user[32] = "";
int logged_in = 0, role = -1;

unsigned int hash_pass(const char* s) {
    unsigned int h = 0;
    while(*s) h = h*31 + *s++;
    return h;
}

void log_audit(const char* a, const char* f, const char* r) {
    FILE* log = fopen("audit.log", "a");
    time_t t = time(NULL);
    fprintf(log, "[%s] ACTION:%s | File:%s | Result:%s | User:%s\n", 
            ctime(&t), a, f, r, logged_in?current_user:"none");
    fclose(log);
}

void init_db() {
    for(int i=0; i<MAX_USERS; i++) users[i].active = 0;
    for(int i=0; i<MAX_FILES; i++) files[i].active = 0;
    // Default admin
    strcpy(users[0].user, "admin");
    users[0].hash = hash_pass("admin123");
    users[0].role = 0; users[0].active = 1;
}

int authenticate(const char* u, const char* p) {
    for(int i=0; i<MAX_USERS; i++) {
        if(users[i].active && strcmp(users[i].user, u)==0 && users[i].hash==hash_pass(p)) {
            strcpy(current_user, u); logged_in = 1; role = users[i].role;
            printf("[AUTH] Login successful! Welcome, %s (Role:%s)\n", u, role==0?"Admin":"User");
            log_audit("LOGIN", u, "SUCCESS");
            return 1;
        }
    }
    printf("[AUTH] Invalid credentials!\n");
    log_audit("LOGIN", u, "FAIL");
    return 0;
}

int find_file(const char* name) {
    for(int i=0; i<MAX_FILES; i++) if(files[i].active && strcmp(files[i].name, name)==0) return i;
    return -1;
}

unsigned short make_perms(int or, int ow, int ox, int gr, int gw, int gx, int otr, int otw, int otx) {
    return (or<<8)|(ow<<7)|(ox<<6)|(gr<<5)|(gw<<4)|(gx<<3)|(otr<<2)|(otw<<1)|otx;
}

void print_perms(unsigned short p) {
    printf("Owner:%c%c%c ", (p>>8)&4?'r':'-', (p>>7)&2?'w':'-', (p>>6)&1?'x':'-');
    printf("Group:%c%c%c ", (p>>5)&4?'r':'-', (p>>4)&2?'w':'-', (p>>3)&1?'x':'-');
    printf("Other:%c%c%c\n", (p>>2)&4?'r':'-', (p>>1)&2?'w':'-', p&1?'x':'-');
}

int create_file(const char* name, unsigned short perms) {
    if(!logged_in) { printf("[ERROR] Login required!\n"); return 0; }
    for(int i=0; i<MAX_FILES; i++) {
        if(!files[i].active) {
            strcpy(files[i].name, name);
            strcpy(files[i].owner, current_user);
            files[i].perms = perms; files[i].enc = 0; files[i].active = 1;
            printf("[FILE] Created '%s' by %s\n", name, current_user);
            print_perms(perms);
            log_audit("CREATE", name, "SUCCESS");
            return 1;
        }
    }
    return 0;
}

int write_file(const char* name, const char* content) {
    int idx = find_file(name);
    if(idx < 0) { printf("[ERROR] File not found!\n"); return 0; }
    if(strcmp(files[idx].owner, current_user)!=0 && !((files[idx].perms>>4)&2)) {
        printf("[ERROR] Permission denied!\n");
        log_audit("WRITE", name, "DENIED");
        return 0;
    }
    FILE* f = fopen(name, "w");
    if(f) { fprintf(f, "%s", content); fclose(f); }
    printf("[FILE] Wrote to '%s'\n", name);
    log_audit("WRITE", name, "SUCCESS");
    return 1;
}

int read_file(const char* name) {
    int idx = find_file(name);
    if(idx < 0) { printf("[ERROR] File not found!\n"); return 0; }
    if(strcmp(files[idx].owner, current_user)!=0 && !((files[idx].perms>>2)&4)) {
        printf("[ERROR] Permission denied!\n");
        log_audit("READ", name, "DENIED");
        return 0;
    }
    FILE* f = fopen(name, "r");
    if(f) {
        char buf[1024];
        printf("[FILE] Content of '%s':\n", name);
        while(fgets(buf, 1024, f)) printf("  %s", buf);
        fclose(f);
    }
    log_audit("READ", name, "SUCCESS");
    return 1;
}

void encrypt_file(const char* name) {
    FILE* f = fopen(name, "r+b");
    if(!f) { printf("[ERROR] Cannot open file!\n"); return; }
    int c;
    while((c=fgetc(f))!=EOF) { fseek(f,-1,SEEK_CUR); fputc(c^KEY,f); }
    fclose(f);
    int idx = find_file(name);
    if(idx>=0) files[idx].enc = 1;
    printf("[SECURITY] File '%s' encrypted\n", name);
    log_audit("ENCRYPT", name, "SUCCESS");
}

void list_files() {
    printf("\n--- FILE LISTING ---\n");
    for(int i=0; i<MAX_FILES; i++) {
        if(files[i].active) {
            printf("  %s | Owner:%s | Enc:%s | ", files[i].name, files[i].owner, files[i].enc?"YES":"NO");
            print_perms(files[i].perms);
        }
    }
}

void view_audit() {
    FILE* f = fopen("audit.log", "r");
    if(!f) { printf("[ERROR] No audit log found!\n"); return; }
    printf("\n--- AUDIT LOG ---\n");
    char buf[256];
    while(fgets(buf, 256, f)) printf("  %s", buf);
    fclose(f);
}

int main() {
    printf("============================================================\n");
    printf("  ST5004CEM - Task 3: Secure File Management System\n");
    printf("============================================================\n\n");

    init_db();

    // Demo: Login as admin
    printf("[DEMO] Logging in as admin...\n");
    authenticate("admin", "admin123");

    // Create files with different permissions
    printf("\n[DEMO] Creating files...\n");
    create_file("report.txt", make_perms(1,1,0, 1,0,0, 0,0,0));
    create_file("secret.dat", make_perms(1,1,0, 0,0,0, 0,0,0));
    create_file("public.txt", make_perms(1,1,0, 1,1,0, 1,0,0));

    // Write content
    printf("\n[DEMO] Writing to files...\n");
    write_file("report.txt", "This is a confidential report.");
    write_file("public.txt", "This is public information.");

    // Read files
    printf("\n[DEMO] Reading files...\n");
    read_file("report.txt");
    read_file("public.txt");

    // Encrypt sensitive file
    printf("\n[DEMO] Encrypting sensitive file...\n");
    encrypt_file("secret.dat");

    // List all files
    list_files();

    // View audit log
    printf("\n");
    view_audit();

    printf("\n[SYSTEM] Demo completed successfully\n");
    return 0;
}
