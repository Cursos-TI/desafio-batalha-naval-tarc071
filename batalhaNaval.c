#include <stdio.h>
#include <string.h>

#define N 10          // tamanho do tabuleiro (10x10)
#define MASK 7        // tamanho das matrizes de habilidade (ímpar facilita centralizar)
#define WATER 0
#define SHIP  3
#define AOE   5

// -----------------------------------------
// Utilidades
// -----------------------------------------

// Zera uma matriz rows x cols com 0.
void clearIntGrid(int rows, int cols, int grid[rows][cols]) {
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
            grid[i][j] = 0;
}

// Imprime o tabuleiro combinando tabuleiro base e overlay de habilidade.
// Precedência visual: se overlay marcar (1), imprime 5; senão imprime o valor do tabuleiro (0 água, 3 navio).
void printBoardWithOverlay(int board[N][N], int overlay[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int v = (overlay[i][j] == 1) ? AOE : board[i][j];
            printf("%d%s", v, (j == N - 1) ? "" : " ");
        }
        printf("\n");
    }
}

// -----------------------------------------
// Construção dinâmica das máscaras (MASK x MASK)
// Cada célula recebe 1 = afetada, 0 = não afetada
// -----------------------------------------

// CONE apontando para baixo:
// O "topo" (origem) fica na primeira linha da máscara.
// A largura cresce conforme descemos as linhas.
// Ex.: para MASK=7, na linha r a meia-largura cresce de 0 até r (limitada pelas bordas).
void buildConeMask(int mask[MASK][MASK]) {
    clearIntGrid(MASK, MASK, mask);
    int apexCol = MASK / 2; // topo centralizado
    for (int r = 0; r < MASK; r++) {
        // meia-largura do cone na linha r (cresce 1 por linha)
        int half = r;
        int cStart = apexCol - half;
        int cEnd   = apexCol + half;
        // mantém dentro dos limites
        if (cStart < 0) cStart = 0;
        if (cEnd >= MASK) cEnd = MASK - 1;
        for (int c = cStart; c <= cEnd; c++) {
            // Condicional garante formato em "V" expandindo para baixo.
            mask[r][c] = 1;
        }
    }
}

// CRUZ com origem no centro:
// Preenche a linha central e a coluna central com 1.
void buildCrossMask(int mask[MASK][MASK]) {
    clearIntGrid(MASK, MASK, mask);
    int mid = MASK / 2;
    for (int r = 0; r < MASK; r++) {
        for (int c = 0; c < MASK; c++) {
            if (r == mid || c == mid) mask[r][c] = 1;
        }
    }
}

// OCTAEDRO (losango) com origem no centro:
// Usa distância Manhattan do centro: |dr| + |dc| <= raio
void buildOctaMask(int mask[MASK][MASK]) {
    clearIntGrid(MASK, MASK, mask);
    int mid = MASK / 2;
    int radius = mid; // para MASK=7, radius=3
    for (int r = 0; r < MASK; r++) {
        for (int c = 0; c < MASK; c++) {
            int dr = (r > mid) ? (r - mid) : (mid - r);
            int dc = (c > mid) ? (c - mid) : (mid - c);
            if (dr + dc <= radius) mask[r][c] = 1;
        }
    }
}

// -----------------------------------------
// Sobreposição de uma máscara (MASK x MASK) ao overlay (N x N)
// Centraliza a máscara em (originRow, originCol) do tabuleiro.
// Mantém limites do tabuleiro via condicionais.
// overlay marca 1 onde a habilidade afeta; não altera o tabuleiro base.
// -----------------------------------------
void applyMaskAt(int overlay[N][N], int originRow, int originCol, int mask[MASK][MASK]) {
    int mid = MASK / 2; // centro da máscara
    for (int r = 0; r < MASK; r++) {
        for (int c = 0; c < MASK; c++) {
            if (mask[r][c] == 1) {
                // Traduz coordenadas da máscara para o tabuleiro, centrando no ponto de origem
                int br = originRow + (r - mid);
                int bc = originCol + (c - mid);
                // Condicionais de limite
                if (br >= 0 && br < N && bc >= 0 && bc < N) {
                    overlay[br][bc] = 1; // marca área afetada
                }
            }
        }
    }
}

// -----------------------------------------
// Exemplo: coloca alguns navios fixos no tabuleiro (valor 3)
// -----------------------------------------
void placeExampleShips(int board[N][N]) {
    // Um navio horizontal de 3 células na linha 2 (índice 2), colunas 1..3
    for (int c = 1; c <= 3; c++) board[2][c] = SHIP;

    // Um navio vertical de 4 células na coluna 7 (índice 7), linhas 5..8 (com corte no limite)
    for (int r = 5; r < N && r < 9; r++) board[r][7] = SHIP;

    // Uma “canoa” isolada
    board[8][2] = SHIP;
}

// -----------------------------------------
// Programa principal
// -----------------------------------------
int main(void) {
    // Tabuleiro base (água e navios)
    int board[N][N];
    clearIntGrid(N, N, board);
    placeExampleShips(board);

    // Overlay para habilidades (separado do tabuleiro base)
    int overlay[N][N];
    clearIntGrid(N, N, overlay);

    // Constrói máscaras
    int cone[MASK][MASK], cross[MASK][MASK], octa[MASK][MASK];
    buildConeMask(cone);
    buildCrossMask(cross);
    buildOctaMask(octa);

    // Define pontos de origem (linha, coluna) para cada habilidade no tabuleiro
    // (pode ajustar livremente; estão fixos conforme simplificações do enunciado)
    int coneOriginRow = 1, coneOriginCol = 3;   // cone centrado perto do topo, “apontando” para baixo
    int crossOriginRow = 5, crossOriginCol = 5; // cruz no meio
    int octaOriginRow = 7, octaOriginCol = 2;   // octaedro (losango) mais abaixo à esquerda

    // Aplica máscaras ao overlay (centralizadas no ponto de origem)
    applyMaskAt(overlay, coneOriginRow,  coneOriginCol,  cone);
    applyMaskAt(overlay, crossOriginRow, crossOriginCol, cross);
    applyMaskAt(overlay, octaOriginRow,  octaOriginCol,  octa);

    // Exibe o tabuleiro resultante (0 água, 3 navio, 5 área afetada)
    // Observação: se uma célula tiver navio e for alcançada pela habilidade,
    // ela será mostrada como 5 (prioridade visual para a área afetada).
    printBoardWithOverlay(board, overlay);

    // Para testes: descomente abaixo para imprimir as máscaras 
    /*
    printf("\nMask CONE:\n");
    for (int r = 0; r < MASK; r++) { for (int c = 0; c < MASK; c++) printf("%d ", cone[r][c]); printf("\n"); }
    printf("\nMask CRUZ:\n");
    for (int r = 0; r < MASK; r++) { for (int c = 0; c < MASK; c++) printf("%d ", cross[r][c]); printf("\n"); }
    printf("\nMask OCTAEDRO:\n");
    for (int r = 0; r < MASK; r++) { for (int c = 0; c < MASK; c++) printf("%d ", octa[r][c]); printf("\n"); }
    */

    return 0;
}
