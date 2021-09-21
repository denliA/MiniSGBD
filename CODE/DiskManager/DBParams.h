typedef struct {
    char* DBPath;//4096 = taille max d'un path linux
    unsigned int pageSize;
    char maxPagesPerFile;
}DBParams;

DBParams params;
