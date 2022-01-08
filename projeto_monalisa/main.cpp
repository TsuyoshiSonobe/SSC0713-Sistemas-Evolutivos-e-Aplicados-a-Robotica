#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

#define nGer 5000  // Numero de geracoes
#define nInd 50     // Numero de individuos em uma geracao

int main(){

    Mat img = imread("225.png", IMREAD_COLOR);

    Mat imgGrayScale;
    cvtColor(img, imgGrayScale, COLOR_BGR2GRAY);

    Mat black;
    inRange(imgGrayScale, 127, 255, black);  

    // Visualizar imagem original
    /*namedWindow("Original", WINDOW_NORMAL);
    namedWindow("Gray Scale", WINDOW_NORMAL);
    namedWindow("Black", WINDOW_NORMAL);
    imshow("Original", img);
    imshow("Gray Scale", imgGrayScale);
    imshow("Black", black);*/
    //cout << "Altura: " << img.size().height << endl;
    //cout << "Largura: " << img.size().width << endl;

    // Transformando Mat em array
    vector<uchar> array;
    if (black.isContinuous()) {
        array.assign(black.data, black.data + black.total()*black.channels());
    }
    else {
        for (int i = 0; i < black.rows; ++i) {
            array.insert(array.end(), black.ptr<uchar>(i), black.ptr<uchar>(i)+black.cols*black.channels());
        }
    }

    int *referencia = new int[array.size()];
    copy(array.begin(), array.end(), referencia);

    for(int i = 0; i < array.size(); i++){
        if(referencia[i] == 255){
            referencia[i] = 1;
        }
    } 

    // Algoritmo Genetico
    const int nGene = array.size();                     // numero de genes por individuo
    const int nAvaliacoes = 625;                        // numero de avaliacoes realizadas (mudar de acordo com o tamanho da imagem)
    const int nGeneAvaliados = nGene/nAvaliacoes;       // numero de genes avaliados por vez (algo entre 100 e 200 eh provavalemente o melhor)
    int fitMedioGer = 0, fitGer = 0, fitInd = 0;        // fitness medio e total da geracao e fitness de individuo
    int fitMInd = -1, mInd = -1;                        // fitness e numero do melhor individuo
    int fitPInd = nGene, pInd = -1;                     // fitness e numero do pior individuo
    int *pop = new int[nInd * nGene];                   // matriz de individuos de uma geracao (populacao)
    int *popAtual = new int[nInd * nGeneAvaliados];     // matriz de individuos com parte dos genes (os que estao sendo avaliados)
    int tamImgFinal = sqrt(array.size());               // tamanho das dimensoes da imagem final (imagem quadrada)
    uint8_t *imgFinal = new uint8_t[tamImgFinal * tamImgFinal]; // matriz que armazenara a imagem final
    Mat finalImg;

    // Criacao da populacao inicial
    srand(time(NULL));    
    for(int i = 0; i < nInd; i++){          
        for(int j = 0; j < nGene; j++){
            pop[i * nGene + j] = rand() % 2;
        }
    }
    
    int aux = 0, atual = 0;
    for(int n = 0; n < nAvaliacoes; n++){

        for(int i = 0; i < nInd; i++){
            aux = nGeneAvaliados * n;          
            for(int j = 0; j < nGeneAvaliados; j++){
                popAtual[i * nGeneAvaliados + j] = pop[i * nGene + aux];
                aux++;
            }
        }
        
        // Passando pelas geracoes
        for(int g = 0; g < nGer; g++){    

            // Imprimindo a populacao da geracao
            //cout << "Geração " << g << endl;
            /*for(int i = 0; i < nInd; i++){
                cout << "Individuo " << i << endl;
                for(int j = 0; j < nGene; j++){
                    cout << pop[i * nGene + j] << " ";
                }
                cout << endl;
            }*/
            
            // Avaliacao e selecao da populacao da geracao
            for(int i = 0; i < nInd; i++){           // passando por cada individuo
                aux = nGeneAvaliados * n;
                for(int j = 0; j < nGeneAvaliados; j++){      // passando por cada gene do individuo
                    if(popAtual[i * nGeneAvaliados + j] == referencia[aux]){
                        fitGer++;
                        fitInd++;
                    }
                    aux++;
                }
            //    cout << "Fitness do individuo " << i << ": " << fitInd << endl;
                if(fitInd > fitMInd){
                    fitMInd = fitInd;
                    mInd = i;
                }
                if(fitInd < fitPInd){
                    fitPInd = fitInd;
                    pInd = i;
                }
                fitInd = 0;
            }

            // Croosover
            int corte = rand() % nGeneAvaliados;
            for(int i = 0; i < nInd; i++){
                for(int j = 0; j < corte; j++){
                    popAtual[i * nGeneAvaliados + j] = popAtual[mInd * nGeneAvaliados + j];
                }
            }
            
            // Mutacao
            int pontoMutacao;
            if (rand()%11 < 7) {                     // probabilidade de mutacao
                for(int i = 0; i < nInd; i++){
                    if(i != mInd){                                  // nao muta o melhor individuo
                        pontoMutacao = rand() % nGeneAvaliados;            // muta um gene aleatorio
                        if(popAtual[i * nGeneAvaliados + pontoMutacao] == 1){
                            popAtual[i * nGeneAvaliados + pontoMutacao] = 0;
                        }
                        else{
                            popAtual[i * nGeneAvaliados + pontoMutacao] = 1;
                        }
                    }
                }
            }

            // Trocando o pior individuo pelo melhor
            for(int j = 0; j < nGeneAvaliados; j++){
                popAtual[pInd * nGeneAvaliados + j] = popAtual[mInd * nGeneAvaliados + j];
            }

            // Resultados da geracao
            /*cout << "Numero do melhor individuo da geracao: " << mInd << " -> Fitness: " << fitMInd << endl;
            fitMedioGer = (fitGer/nInd);       
            cout << "Fitness medio da geração: " << fitMedioGer << endl;
            fitGer = 0; 
            cout << endl;*/
            
            // Coloca os individuos da populacao atual na matriz populacao
            for(int i = 0; i < nInd; i++){ 
                aux = nGeneAvaliados*n;         
                for(int j = 0; j < nGeneAvaliados; j++){
                    pop[i * nGene + aux] = popAtual[i * nGeneAvaliados + j];
                    aux++;
                }
            }

            // Transforma o array binario em matriz de cores (branco e preto, por enquanto)
            int k = 0;
            for(int i = 0; i < tamImgFinal; i++){
                for(int j = 0; j < tamImgFinal; j++){
                    imgFinal[i * tamImgFinal + j] = pop[mInd * nGene + k];
                    k++;
                    if(imgFinal[i * tamImgFinal + j] == 1)
                        imgFinal[i * tamImgFinal + j] = 255;
                }
            }

            // Transforma a matriz em imagem e mostra
            finalImg = Mat(tamImgFinal, tamImgFinal, CV_8U, imgFinal);
            hconcat(black, finalImg, finalImg);
            namedWindow("Imagem Final", WINDOW_NORMAL);
            imshow("Imagem Final", finalImg);
            waitKey(1);
            
            // Encerra antes caso o melhor individuo seja o ideal
            if(fitMInd == nGeneAvaliados){
                break;
            }
        }

        // Copia os genes avaliados do melhor individuo para todos os outros
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
        fitMInd = -1; 
        mInd = -1;                        // fitness e numero do melhor individuo
        fitPInd = nGene;
        pInd = -1;                     // fitness e numero do pior individuo

    }

    waitKey(0);
    destroyAllWindows();

    delete [] imgFinal;
    delete [] referencia;

    return 0;
}
