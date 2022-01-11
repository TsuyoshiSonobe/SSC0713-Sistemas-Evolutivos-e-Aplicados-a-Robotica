#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

#define nGer 10000          // Numero de geracoes
#define nInd 50             // Numero de individuos em uma geracao
#define taxMut 1            // Numero de mutacoes por individuo
#define valMut 5            // Valor somado ou subtraido a cada pixel mutado

int main(){

    Mat img = imread("monalisa.jpg", IMREAD_COLOR);
    resize(img, img, Size(100, 100), INTER_LINEAR);   // Redefinindo tamanho da imagem

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
    imshow("Black", black);
    waitKey(0);
    destroyAllWindows();*/

    // Transformando Mat em array
    vector<uchar> array;
    if (black.isContinuous()) {
        array.assign(imgGrayScale.data, imgGrayScale.data + imgGrayScale.total()*imgGrayScale.channels());
    }
    else {
        for (int i = 0; i < black.rows; ++i) {
            array.insert(array.end(), imgGrayScale.ptr<uchar>(i), imgGrayScale.ptr<uchar>(i)+imgGrayScale.cols*imgGrayScale.channels());
        }
    }

    int *referencia = new int[array.size()];
    copy(array.begin(), array.end(), referencia);

    /*for(int i = 0; i < array.size(); i++){
        if(referencia[i] == 255){
            referencia[i] = 1;
        }
    } */

    // Algoritmo Genetico
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
    Mat finalImg;

    // Criacao da populacao inicial
    srand(time(NULL));    
    for(int i = 0; i < nInd; i++){          
        for(int j = 0; j < nGene; j++){
            pop[i * nGene + j] = rand() % 256;
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
                    fitInd += -1*abs(popAtual[i * nGeneAvaliados + j]-referencia[aux]);
                    aux++;
                    fitGer += fitInd;
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
            int pontoMutacao, operacao;
            //if (rand()%11 < 7) {                     // probabilidade de mutacao
                for(int i = 0; i < nInd; i++){
                    if(i != mInd){                                  // nao muta o melhor individuo
                        for(int k = 0; k < taxMut; k++){
                            pontoMutacao = rand() % nGeneAvaliados;            // muta um gene aleatorio
                            operacao = rand() % 2;
                            if(operacao == 0){
                                if(popAtual[i * nGeneAvaliados + pontoMutacao] + valMut < 256)
                                    popAtual[i * nGeneAvaliados + pontoMutacao] += valMut;
                            }
                            else{
                                if(popAtual[i * nGeneAvaliados + pontoMutacao] - valMut > 0)
                                    popAtual[i * nGeneAvaliados + pontoMutacao] -= valMut;
                            }
                        }
                    }
                }
            //}
            
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
            hconcat(imgGrayScale, finalImg, finalImg);
            namedWindow("Imagem Final", WINDOW_NORMAL);
            imshow("Imagem Final", finalImg);
            waitKey(1);
            
            // Encerra antes caso o melhor individuo seja o ideal
            if(fitMInd > (nGeneAvaliados * (-1) - 11)){
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
        fitMInd = -1000000; 
        mInd = -1;                     // fitness e numero do melhor individuo
        fitPInd = nGene;
        pInd = -1;                     // fitness e numero do pior individuo

    }

    waitKey(0);
    destroyWindow("finalImg");

    delete [] imgFinal;
    delete [] referencia;

    return 0;
}
