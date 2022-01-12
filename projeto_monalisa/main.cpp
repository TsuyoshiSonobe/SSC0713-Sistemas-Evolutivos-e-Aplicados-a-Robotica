#include <iostream>
#include <stdio.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

#define nGer 10000          // Numero de geracoes
#define nInd 50             // Numero de individuos em uma geracao
#define taxMut 1            // Numero de mutacoes por individuo
#define valMut 5            // Valor somado ou subtraido a cada pixel mutado

int main(){

    Mat img = imread("monalisa.jpg", IMREAD_COLOR);     // Leitura da imagem
    resize(img, img, Size(100, 100), INTER_LINEAR);     // Redefinindo tamanho da imagem

    // Alterando imagem de RGB (colorida) para Gray Scale (escala de cinza)
    Mat imgGrayScale;
    cvtColor(img, imgGrayScale, COLOR_BGR2GRAY);

    // Mostrando imagem original
    namedWindow("Original", WINDOW_NORMAL);
    imshow("Original", img);

    // Linearizando a imagem, ou seja, transformamos de matriz para array,
    // colocando os valores dos pixels da imagem em um array que sera usado como referencia para comparacao e calculo do fitness
    vector<uchar> array;
    if (imgGrayScale.isContinuous()) {
        array.assign(imgGrayScale.data, imgGrayScale.data + imgGrayScale.total()*imgGrayScale.channels());
    }
    else {
        for (int i = 0; i < imgGrayScale.rows; ++i) {
            array.insert(array.end(), imgGrayScale.ptr<uchar>(i), imgGrayScale.ptr<uchar>(i)+imgGrayScale.cols*imgGrayScale.channels());
        }
    }
    int *referencia = new int[array.size()];
    copy(array.begin(), array.end(), referencia);

    // Abrindo arquivo para escrita dos dados do algoritmo evolutivo
    FILE *file1;
    file1 = fopen("dados.txt", "w");
    if(file1 == NULL){
        printf("Erro na abertura do arquivo");
        return 1;
    }

    
    // Algoritmo Evolutivo
    const int nGene = array.size();                     // numero de genes por individuo
    const int nAvaliacoes = 2000;                       // numero de avaliacoes realizadas
    const int nGeneAvaliados = nGene/nAvaliacoes;       // numero de genes avaliados por vez
    int fitMedioGer = 0, fitGer = 0, fitInd = 0;        // fitness medio e total da geracao e fitness de individuo
    int fitMInd = -1000000, mInd = -1;                  // fitness e numero do melhor individuo
    int fitPInd = nGene, pInd = -1;                     // fitness e numero do pior individuo
    int *pop = new int[nInd * nGene];                   // matriz de individuos de uma geracao (populacao)
    int *popAtual = new int[nInd * nGeneAvaliados];     // matriz de individuos com parte dos genes (os que estao sendo avaliados)
    int tamImgFinal = sqrt(array.size());               // tamanho das dimensoes da imagem final (imagem quadrada)
    uint8_t *imgFinal = new uint8_t[tamImgFinal * tamImgFinal]; // matriz que armazenara a imagem final
    Mat finalImg;                                       // objeto que armazenara a imagem final para ser mostrada

    // Criacao da populacao inicial
    srand(time(NULL));    
    // gera uma matriz na qual o numero de linhas eh o numero de individuos da geracao
    // e o numero de colunas eh o numero de genes (quantidade de pixels da imagem utilizada),
    // depois inicia a matriz com valores aleatorios de 0 a 255
    for(int i = 0; i < nInd; i++){          
        for(int j = 0; j < nGene; j++){
            pop[i * nGene + j] = rand() % 256;  
        }
    }
    
    // Separamos a analise da imagem em partes, utilizando o algoritmo evolutivo para avaliar uma quantidade de pixels 
    // definida por nGeneAvaliados de cada vez, e repetindo o processo por nAvaliacoes vezes ate que toda a imagem seja avaliada
    int aux = 0, atual = 0;
    for(int n = 0; n < nAvaliacoes; n++){
        
        // Coloca os genes (pixels) que serao avaliados em popAtual
        // (dessa forma conseguimos fazer alteracoes sem modificar os outros genes da populacao)
        for(int i = 0; i < nInd; i++){
            aux = nGeneAvaliados * n;          
            for(int j = 0; j < nGeneAvaliados; j++){
                popAtual[i * nGeneAvaliados + j] = pop[i * nGene + aux];
                aux++;
            }
        }
        
        // Passando pelas geracoes
        for(int g = 0; g < nGer; g++){    

            // Avaliacao e selecao da populacao da geracao

            // Passando por cada individuo
            for(int i = 0; i < nInd; i++){           
                aux = nGeneAvaliados * n;
                // Passando por cada gene do individuo
                for(int j = 0; j < nGeneAvaliados; j++){      
                    fitInd += -1*abs(popAtual[i * nGeneAvaliados + j]-referencia[aux]);
                    aux++;
                    fitGer += fitInd;
                }
                // Melhor individuo
                if(fitInd > fitMInd){
                    fitMInd = fitInd;
                    mInd = i;
                }
                // Pior individuo
                if(fitInd < fitPInd){
                    fitPInd = fitInd;
                    pInd = i;
                }
                fitInd = 0;
            }
            
            // Crossover
            // (copiamos uma parte de tamanho aleatorio do melhor individuo para os demais)
            int corte = rand() % nGeneAvaliados;
            for(int i = 0; i < nInd; i++){
                for(int j = 0; j < corte; j++){
                    popAtual[i * nGeneAvaliados + j] = popAtual[mInd * nGeneAvaliados + j];
                }
            }
            
            // Mutacao
            // (muta um certo numero de genes de todos os individuos, exceto do melhor, somando ou subtraindo um certo valor)
            int pontoMutacao, operacao;
            for(int i = 0; i < nInd; i++){
                if(i != mInd){                                  // nao muta o melhor individuo
                    for(int k = 0; k < taxMut; k++){            // muta o numero de genes definido por taxMut 
                        pontoMutacao = rand() % nGeneAvaliados; // muta um gene aleatorio
                        operacao = rand() % 2;
                        if(operacao == 0){
                            // Soma valMut ao gene, caso o valor da soma nao ultrapasse 255
                            if(popAtual[i * nGeneAvaliados + pontoMutacao] + valMut < 256)
                                popAtual[i * nGeneAvaliados + pontoMutacao] += valMut;  
                        }
                        else{
                            // Subtrai valMut do gene, caso o valor da subtracao seja maior que zero
                            if(popAtual[i * nGeneAvaliados + pontoMutacao] - valMut > 0)
                                popAtual[i * nGeneAvaliados + pontoMutacao] -= valMut;  
                        }
                    }
                }
            }
            
            // Trocando o pior individuo pelo melhor
            for(int j = 0; j < nGeneAvaliados; j++){
                popAtual[pInd * nGeneAvaliados + j] = popAtual[mInd * nGeneAvaliados + j];
            }

            // Resultados da geracao
            fitMedioGer = (fitGer/nInd);       
            fitGer = 0; 
            // Gravando a geração, melhor indivíduo, nota dele e a nota média da geração
            fprintf(file1,"%d,%d,%d,%d\n", g ,mInd, fitMInd, fitMedioGer);
            
            // Coloca os individuos da populacao atual na matriz que contem toda a populacao com todos os genes
            for(int i = 0; i < nInd; i++){ 
                aux = nGeneAvaliados*n;         
                for(int j = 0; j < nGeneAvaliados; j++){
                    pop[i * nGene + aux] = popAtual[i * nGeneAvaliados + j];
                    aux++;
                }
            }

            // Utiliza o melhor individuo da geracao para gerar a imagem final,
            // mudando do vetor com os valores dos pixels para a matriz que gera a imagem
            int k = 0;
            for(int i = 0; i < tamImgFinal; i++){
                for(int j = 0; j < tamImgFinal; j++){
                    imgFinal[i * tamImgFinal + j] = pop[mInd * nGene + k];
                    k++;
                }
            }

            // Transforma a matriz em imagem e mostra
            finalImg = Mat(tamImgFinal, tamImgFinal, CV_8U, imgFinal);
            hconcat(imgGrayScale, finalImg, finalImg);
            namedWindow("Imagem Final", WINDOW_NORMAL);
            imshow("Imagem Final", finalImg);
            waitKey(1);
            
            // Encerra antes caso o melhor individuo esteja proximo do ideal
            // (adicionamos um certo limite para que os pixels nao precisem todos terem exatamente os mesmos valores dos originais,
            // mas que sejam proximos o suficiente para que a imagem reconstruida seja muito parecida com a original)
            if(fitMInd > (nGeneAvaliados * (-1) - 11)){
                break;
            }
        }

        // Copia os genes avaliados do melhor individuo para todos os outros
        // (isso faz com que os pixels ja avaliados e escolhidos sejam iguais para todos da populacao)
        for(int i = 0; i < nInd; i++){
            for(int j = 0; j < nGeneAvaliados; j++){
                popAtual[i * nGeneAvaliados + j] = popAtual[mInd * nGeneAvaliados + j];
            }
        }
        for(int i = 0; i < nInd; i++){ 
            aux = nGeneAvaliados*n;         
            for(int j = 0; j < nGeneAvaliados; j++){
                pop[i * nGene + aux] = popAtual[i * nGeneAvaliados + j];
                aux++;
            }
        }

        // Reseta os valores de melhor e pior individuos
        fitMInd = -1000000;             
        mInd = -1;                     
        fitPInd = nGene;                
        pInd = -1;                     

    }

    // Espera que o usuario pressione uma tecla para fechar a janela da imagem final
    waitKey(0);
    destroyAllWindows();

    // Desalocando a memoria utilizada
    delete [] imgFinal;
    delete [] referencia;
    
    // Fechando arquivo dados.txt
    fclose(file1);

    return 0;
}