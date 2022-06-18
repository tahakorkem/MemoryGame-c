#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>

#define N 8
#define N_0 4
#define N_1 6
#define N_2 8
#define M 16
#define M_0 2
#define M_1 6
#define M_2 16
#define COLOR_CYAN "\x1b[36m"
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

int scanDifficulty(void);

void shuffleCards(char[N][N], int);

void removeFromMemory(int, int[M], char[M], int);

void addIntoMemory(int, char, int[M], char[M], int);

int generateRandomCoord(int[N][N], int[M], int, int);

int findOneOfPairInMemory(int[M], char[M], int);

int findPairOfCardInMemory(char, int[M], char[M], int);

int isCoordInMemory(int, int[M], int);

int findCoordIndexInMemory(int, int[M], int);

void parseRowColFromCoord(int, int *, int *, int);

int scanCoord(int[N][N], int);

void printTable(char[N][N], int[N][N], int, int);

void printMemory(int[M], char[M], int);

int main(void) {
    srand(time(NULL));

    char cards[N][N]; // kartların bulunduğu iki boyutlu dizi
    int flags[N][N]; // kartların anlık durumu; 0 -> kapalı, 1 -> açık, -1 -> eşleştirilmiş
    int i, j;
    int difficulty; // zorluk seviyesi; 0 -> kolay, 1 -> orta, 2 -> zor
    int n, m; // dinamik dizi uzunlukları; n: oyun alanı kenar uzunluğu, m: bilgisayar hafızasının uzunluğu
    int isGameFinished = 0; // oyunun bitip bitmediği

    int turn = 0; // sıranın kimde olduğu; 0 -> kullanıcı, 1 -> bilgisayar

    int scores[2] = {0, 0}; // oyuncu skorları; [0] -> kullanıcı skoru, [1] -> bilgisayar skoru
    int predictions[2] = {0, 0}; // oyuncu tahmin sayıları; [0] -> kullanıcı tahmini, [1] -> bilgisayar tahmini

    int lastCoords[2]; // bir tahmin esnasında açılan iki kartı tutan dizi
    int coordsInComputerMemory[M]; // bilgisayarın koordinat hafızası,
    // koordinat değeri -1 ise o göz boş sayılır
    // koordinatlar satır*kenar_uzunluğu + sütun şeklinde tutulur
    char cardsInComputerMemory[M]; // bilgisayarın kart hafızası
    // koordinat hafızası ile kart hafızası birbirine bağlıdır

    // kullacıdan zorluk girişi alınır
    printf("-----Zorluk Modlari-----\n");
    printf("1 - Kolay Mod\n");
    printf("2 - Orta Mod\n");
    printf("3 - Zor Mod\n");
    difficulty = scanDifficulty();

    system("cls");

    // zorluk ayarlamaları yapılır
    if (difficulty == 0) {
        n = N_0;
        m = M_0;
    } else if (difficulty == 1) {
        n = N_1;
        m = M_1;
    } else {
        n = N_2;
        m = M_2;
    }

    // kartların içerikleri doldurulur
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            cards[i][j] = 60 + (i * n + j + 1) % (n * n / 2);
        }
    }

    // ilk flag değerler 0 (yani kapalı) olarak atanır
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            flags[i][j] = 0;
        }
    }

    // kartlar karılır
    shuffleCards(cards, n);

    // bilgisayarın hafızası başlangıçta boş kılınır
    for (i = 0; i < m; i++) {
        coordsInComputerMemory[i] = -1;
    }

    printf("Kartlarin acik hali su sekilde:\n\n");
    printTable(cards, flags, n, 1);

    printf("Oyuna baslamak icin tusa basin...");
    getch();
    system("cls");

    while (!isGameFinished) {
        printf("   Zorluk modu:\t\t%s\n", difficulty == 0 ? "Kolay" : difficulty == 1 ? "Orta" : "Zor");
        printf("   Skor:\t\t[Oyuncu] %d - %d [Bilgisayar]\n", scores[0], scores[1]);
        printf("   Tahmin sayisi:\t[Oyuncu] %d - %d [Bilgisayar]\n", predictions[0], predictions[1]);
        printf("   Oynayan:\t\t%s\n", turn == 0 ? "Oyuncu" : "Bilgisayar");
        printf("   Bilgisayar hafizasi:\t");
        printMemory(coordsInComputerMemory, cardsInComputerMemory, m);
        printf("\n\n");

        printTable(cards, flags, n, 0);

        for (i = 0; i < 2; i++) {
            int selectedCoord, row, col;
            if (turn == 0) {
                // hamleyi yapan kullanıcı ise
                selectedCoord = scanCoord(flags, n);
            } else {
                // bilgisayar ise
                printf("Devam etmek icin tusa basin...");
                getch();

                if (i == 0) {
                    // eğer ilk kart açılacaksa
                    selectedCoord = findOneOfPairInMemory(coordsInComputerMemory, cardsInComputerMemory, m);
                    // eşi olan bir kart seçildiyse hafızdan çıkarılır
                    // ikinci adımda yine kendisi değil de eşi bulunsun diye
                    if (selectedCoord != -1) {
                        removeFromMemory(selectedCoord, coordsInComputerMemory, cardsInComputerMemory, m);
                    }
                } else {
                    // eğer ikinci kart açılacaksa
                    int previousCoord = lastCoords[0];
                    parseRowColFromCoord(previousCoord, &row, &col, n);
                    selectedCoord = findPairOfCardInMemory(cards[row][col], coordsInComputerMemory,
                                                           cardsInComputerMemory, m);
                }
                // ilk ya da ikinci kart açılışında açmak için hafızadan bir koordinat bulunamadıysa
                // rastgele bir değer seçilir
                if (selectedCoord == -1) {
                    selectedCoord = generateRandomCoord(flags, coordsInComputerMemory, n, m);
                }

            }

            // bu adımda seçilen kartın koordinatları row ve col değerine atılır
            parseRowColFromCoord(selectedCoord, &row, &col, n);
            // seçilen kart açılır
            flags[row][col] = 1;

            // seçilen koordinat lastCoords dizisinde tutulur
            lastCoords[i] = selectedCoord;

            printf("\n\n");
            printTable(cards, flags, n, 0);
            printf("\n");
        }

        int coord1 = lastCoords[0], row1, col1;
        parseRowColFromCoord(coord1, &row1, &col1, n);
        char card1 = cards[row1][col1];

        int coord2 = lastCoords[1], row2, col2;
        parseRowColFromCoord(coord2, &row2, &col2, n);
        char card2 = cards[row2][col2];

        // tahmin sayısı artırılır
        predictions[turn]++;

        if (card1 == card2) {
            // eş kartlar açılmışsa
            // kartlar eşleştirilmiş olarak işaretlenir
            flags[row1][col1] = -1;
            flags[row2][col2] = -1;

            // bu koordinatlar hafızadan çıkarılır
            removeFromMemory(coord1, coordsInComputerMemory, cardsInComputerMemory, m);
            removeFromMemory(coord2, coordsInComputerMemory, cardsInComputerMemory, m);

            // skoru artırılır
            // maksimum skora ulaşıldığında veya tüm kartlar açıldığında oyun biter
            if (++scores[turn] > (n * n) / 4 || scores[0] + scores[1] == (n * n) / 2) {
                isGameFinished = 1;
            }
        } else {
            // eş olmayan kartlar açılmışsa
            // kartlar kapatılır
            flags[row1][col1] = 0;
            flags[row2][col2] = 0;

            // bilgisayar kartları hafızasına alır
            addIntoMemory(coord1, card1, coordsInComputerMemory, cardsInComputerMemory, m);
            addIntoMemory(coord2, card2, coordsInComputerMemory, cardsInComputerMemory, m);

            // kullanıcı sırası değiştirilir
            turn = !turn;
        }

        printf("Devam etmek icin tusa basin...");
        getch();
        system("cls");
    }
    // oyunu kazanan bilgisi verilir
    printf("Oyun bitti!\n\n");
    printf("Nihai Skor: [Oyuncu] %d - %d [Bilgisayar]\n", scores[0], scores[1]);
    if (scores[0] == scores[1]) {
        printf("Oyun berabere bitti.");
    } else {
        printf("%s %d adimda kazandi.", turn == 0 ? "Oyuncu" : "Bilgisayar", predictions[turn]);
    }

    printf("\n");
    return 0;
};

/**
* Verilen koordinatı hafızadan siler
* Koordinat hafıza değilse hiçbir işlem yapmaz
* Değer silindikten sonra sağdaki değerler birer adım sola kaydırılır
* Örneğin; 12:'H' değeri hafızadan silinmek istenirse
* dizinin ilk hali               -> dizinin son hali
* [1: 'A', 12:'H', 2:'F', 9:'J'] -> [1: 'A', 2:'F', 9:'J', -]
*/
void removeFromMemory(int coord, int coordsMemory[M], char cardsMemory[M], int m) {
    int coordIndex = findCoordIndexInMemory(coord, coordsMemory, m);
    if (coordIndex == -1) { return; }
    int i = coordIndex;
    while (i < m - 1 && coordsMemory[i] != -1) {
        coordsMemory[i] = coordsMemory[i + 1];
        cardsMemory[i] = cardsMemory[i + 1];
        i++;
    }
    coordsMemory[i] = -1;
}

/**
* Verilen koordinatı ve kartı bilgisayar hafızasına ekler
* Verilen koordinat zaten hafızada mevcutsa hiçbir işlem yapmaz
* Veriler dizinin başına eklenir ve dizi bir adım sağa kaydırılır
* Örneğin; 5:'C' değeri hafızaya eklenmmek istenirse
* dizinin ilk hali               -> dizinin son hali
* [1: 'A', 12:'H', 2:'F', 9:'J'] -> [5: 'C', 1: 'A', 12:'H', 2:'F']
*/
void addIntoMemory(int coord, char card, int coordsMemory[M], char cardsMemory[M], int m) {
    if (isCoordInMemory(coord, coordsMemory, m)) { return; }
    int i;
    for (i = m - 1; i >= 1; i--) {
        coordsMemory[i] = coordsMemory[i - 1];
        cardsMemory[i] = cardsMemory[i - 1];
    }
    coordsMemory[0] = coord;
    cardsMemory[0] = card;
}

/**
* Oyun sınırları içerisinde rastgele bir koordinat oluşturur
* Seçilen koordinattaki kart kapalı olmalıdır
* Seçilen koordinat hafızada olmamalıdır
* Geçerli koordinatların yer aldığı diziden rastgele bir eleman seçilir
* Koordinat (satır*kenar_uzunluğu + sütun) şeklinde döndürülür
*/
int generateRandomCoord(int flags[N][N], int memory[M], int n, int m) {
    int i, j, k = 0;
    int eligibleCoords[N * N];
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int coord = i * n + j;
            if (flags[i][j] == 0 && !isCoordInMemory(coord, memory, m)) {
                eligibleCoords[k++] = coord;
            }
        }
    }
    int randomIndex = rand() % k;
    return eligibleCoords[randomIndex];
}

/**
* Hafızada birbirine eş kartlar arar
* Eğer bulursa ilk kartın koordinatını döndürür
* Herhangi eş bulamazsa -1 değerini döndürür
* Örneğin;
* [1: 'A', 12:'B', 2:'F', 3:'J', 7: 'B', 0:'H', 10:'A', 17:'X']
* yukarıdaki hafızada ikişer eş kart bulunmakta: {1: 'A' - 10:'A}', {12:'B' - 7: 'B'}
* bu durumda 1: 'A' değerinin koordinatını yani 1 değerini döndürür
*/
int findOneOfPairInMemory(int coordsMemory[M], char cardsMemory[M], int m) {
    int i = 0, j, found = 0;
    while (!found && i < m - 1 && coordsMemory[i] != -1) {
        j = i + 1;
        while (!found && j < m && coordsMemory[j] != -1) {
            if (cardsMemory[i] == cardsMemory[j]) { found = 1; }
            else { j++; }
        }
        if (!found) { i++; }
    }
    if (found) { return coordsMemory[i]; }
    else { return -1; }
}

/**
* Verilen kartın eşini bilgisayarın hafızasında arar
* Eğer hafızada bulursa koordinatı döndürür
* Hafızada bulamazsa -1 değerini döndürür
*/
int findPairOfCardInMemory(char card, int coordsMemory[M], char cardsMemory[M], int m) {
    int i = 0, found = 0;
    while (!found && i < m && coordsMemory[i] != -1) {
        if (card == cardsMemory[i]) { found = 1; }
        else { i++; }
    }
    if (found) { return coordsMemory[i]; }
    else { return -1; }
}

/**
* Verilen koordinatın bilgisayarın hafızasında olup olmadığı
* değerini döndürür
*/
int isCoordInMemory(int coord, int memory[M], int m) {
    return findCoordIndexInMemory(coord, memory, m) != -1;
}

/**
* Verilen koordinatın bilgisayarın hafızasında arar
* Eğer hafızada bulursa koordinatın hafızadaki indisini döndürür
* Hafızada bulamazsa -1 değerini döndürür
*/
int findCoordIndexInMemory(int coord, int memory[M], int m) {
    int i = 0, found = 0;
    while (!found && i < m && memory[i] != -1) {
        if (coord == memory[i]) { found = 1; }
        else { i++; }
    }
    if (found) { return i; }
    else { return -1; }
}

/**
* Kullanıcıdan (satır*kenar_uzunluğu + sütun) formatında koordinat alınır
* Bu koordinattan satır ve sütun indisi çıkarılır
* Dışarıdan adresleri verilen row ve col değişkenlerine değerleri atanır
*/
void parseRowColFromCoord(int coord, int *row, int *col, int n) {
    *row = coord / n;
    *col = coord % n;
}

/**
* Kullanıcıdan koordinat girişi alır
* Seçilen koordinat tablo sınırlarında olmalıdır
* Açık ya da eşleştirilmiş olmamalıdır
* Geçerli giriş alına kadar tekrarlanır
* Koordinat (satır*kenar_uzunluğu + sütun) şeklinde döndürülür
*/
int scanCoord(int flags[N][N], int n) {
    int row, col;
    printf("Koordinat girin: ");
    scanf("%d,%d", &row, &col);

    if (row < 1 || row > n || col < 1 || col > n) {
        printf("Gecerli bir koordinat girmediniz!\n");
        return scanCoord(flags, n);
    }

    row--;
    col--;

    int currFlag = flags[row][col];
    if (currFlag == 1) {
        printf("Secmis oldugunuz hucre zaten acilmis durumda!\n");
        return scanCoord(flags, n);
    } else if (currFlag == -1) {
        printf("Secmis oldugunuz hucre eslestirildigi icin secilemez!\n");
        return scanCoord(flags, n);
    }
    return row * n + col;
}

/**
* Bilgisayarın hafızasını dizi gösteriminde yazdırır
*/
void printMemory(int coordsMemory[M], char cardsMemory[M], int m) {
    int i;
    printf("[");
    for (i = 0; i < m; i++) {
        int coord = coordsMemory[i];
        int card = cardsMemory[i];
        if (coord == -1) { printf("-"); }
        else { printf("%d:'%c'", coord, card); }
        if (i < m - 1) { printf(", "); }
    }
    printf("]");
}

/**
* Kartları tablo biçiminde yazdırır
* ignoreFlags değeri 1 ise flagler dikkate alınmaz, tüm kartlar gösterilir
* ignoreFlags 0 ise flaglere uygun gösterim sağlanır
*/
void printTable(char array[N][N], int flags[N][N], int n, int ignoreFlags) {
    int i, j;
    printf("     ");
    for (i = 0; i < n; i++) {
        printf("%d   ", i + 1);
    }
    printf("\n");
    printf("   +");
    for (i = 0; i < n; i++) {
        printf("---+");
    }
    printf("\n");
    for (i = 0; i < n; i++) {
        printf(" %d |", i + 1);
        for (j = 0; j < n; j++) {
            char flagValue = flags[i][j];
            char arrValue = array[i][j];
            // kart flag durumana göre yazdırılır
            if (ignoreFlags || flagValue == 1) { printf(" %s%c%s |", COLOR_CYAN, arrValue, COLOR_RESET); }
            else if (flagValue == 0) { printf(" * |"); }
            else { printf(" %s-%s |", COLOR_RED, COLOR_RESET); }
        }
        printf("\n");
        printf("   +");
        for (j = 0; j < n; j++) {
            printf("---+");
        }
        printf("\n");
    }
    printf("\n");
}

/**
* Fisher-Yates algoritmasının iki boyutlu diziye uyarlanmış hali
* İki boyutlu kart dizisi karılır
*/
void shuffleCards(char cards[N][N], int n) {
    int i, j;
    for (i = n - 1; i > 0; i--) {
        for (j = n - 1; j > 0; j--) {
            int row = rand() % (i + 1);
            int col = rand() % (j + 1);
            char temp = cards[i][j];
            cards[i][j] = cards[row][col];
            cards[row][col] = temp;
        }
    }
}

/**
* Kullanıcıdan zorluk girişi alır
* Geçerli giriş alına kadar tekrarlanır
*/
int scanDifficulty() {
    int diff;
    printf("Zorluk modu seciniz: ");
    scanf("%d", &diff);

    if (diff < 1 || diff > 3) {
        printf("Gecerli bir secim yapmadiniz!\n");
        return scanDifficulty();
    }
    return diff - 1;
}