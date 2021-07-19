# Monitoramento de qualidade de água

<p align="justify">  A <a href="https://docs.google.com/presentation/d/1uHk2DYrfxsaQ3JNXLhX1uGLU5sidi7bCZawYK-Eue2A/edit?usp=sharing" >qualidade da água</a> pode ser conhecida sabendo determinados parâmetros: temperatura, pH, turbidez, oxigênio dissolvido, condutividade, solidos dissolvidos e entre outros. Esse trabalho tem o objetivo de medir a turbidez, temperatura e pH da água para estimar sua qualidade. </p>

# Implementações

<h2>Implementação Gateway LoRa + Sensor node (pH, temperatura, turbidez)</h2>
<p align="justify">
   <img src="https://github.com/FelipeCamargoXavier/water-quality/blob/main/images/prototipo1.jpg?raw=true"/>
</p>

<p align="justify">
  O sensor node é responsável por medir a temperatura, ph e turbidez. Os dados são enviados através do radio LoRa (915 MHz e SF7) a cada 15s. O Gateway utiliza o protoclo MQTT para publicar na plataforma <a href="https://ubidots.com/">Ubidots</a> as informações recebidas do sensor node.
</p>

<ul>
  <li><a href="https://github.com/FelipeCamargoXavier/water-quality/blob/main/gateway-lora-monitoramento-agua.cpp">Gateway</a></li>
  <li><a href="https://github.com/FelipeCamargoXavier/water-quality/blob/main/end-node-temperatura-ph-turbidez.cpp">Sensor node (ph, temperatura, turbidez)</a></li>
</ul>

<h4>Componentes:</h4>
<ul>
  <li>2 x Heltec ESP32 WiFi LoRa V2</li>
  <li>Sensor de temperatura DFRobot DFR0198 DS18B20 [1]</li>
  <li>Sensor de turbidez DFRobot SEN0189 [2] </li>
   <li>Sensor de pH DFRobot SEN0189 [3] </li>
</ul>

<h2>Implementação com ARDUINO UNO:</h2>

<p align="justify"> Implementação de menor custo que pode ser utilizada para testar os sensores. A principal diferença é que o ESP32 trabalha com 3.3V e o 
  Arduino com 5.0V
</p>

<ul>
  <li><a href="https://github.com/FelipeCamargoXavier/water-quality/blob/main/turbity-temperature.ino">Monitoramento de turbidez e temperatura</a></li>
  <li><a href="https://github.com/FelipeCamargoXavier/water-quality/blob/main/turbity-temperature-ph.ino">Monitoramento de turbidez, temperatura e pH</a></li>
</ul>


<h4>Componentes:</h4>
<ul>
  <li>Arduino Uno</li>
  <li>2 X Resistores 10k</li>
  <li>Sensor de temperatura DFRobot DFR0198 DS18B20 [1]</li>
  <li>Sensor de turbidez DFRobot SEN0189 [2] </li>
   <li>Sensor de pH DFRobot SEN0189 [3] </li>
</ul>



# Sensores (calibração e informações técnicas)

<h3> Sensor pH SEN0161-V2 </h3>

<p align="justify"> 
A biblioteca <a href="https://github.com/DFRobot/DFRobot_PH">DFRobot_PH Library</a>  implementa a medição com compensação de temperatura e a calibração do sensor SEN0189. As referências para calibração são as amostras adquiridas com o kit do sensor. A partir das amostras é obtido dois pontos no gráfico <b>Tensão(V) x pH</b> e traçado uma curva linear que relaciona a tensão com a medida de pH [3]. Para o ESP32 é utilizado a biblioteca <a href="https://github.com/GreenPonik/DFRobot_ESP_PH_BY_GREENPONIK">DFRobot_ESP_PH_BY_GREENPONIK</a>.
</p>

<ul>
  <li>Tensão de alimentação: 3.3~5.5V</li>
  <li>Temperatura de trabalho: 5~60°C</li>
  <li>Acurácia: ±0.1@25℃</li>
</ul>

<h3> Sensor turbidez SEN0189 </h3>

<p>
  O sensor tem a capacidade de medir a turbidez de 0 a 3000 NTU [2]. A equação 1 descreve a relação entre a tensão (V) e a turbidez:
  
          y = -1120.4x² + 5742.3x - 4352.9  (equação 1)
</p>

  <ul>
  <li>Tensão de alimentação: 5.0V</li>
  <li>Tensão de saída: 0-4.5V  *(para o ESP32 é necessário adequar a tensão para 0-3.3V)</li>
  <li>Temperatura de trabalho: 5℃~90℃</li>
  </ul>
  
<h3> Sensor temperatura DSB18B20 </h3>

 <ul>
  <li>Tensão de alimentação: 3.0V to 5.5V</li>
  <li>Temperatura de trabalho: -55 to 125℃ (-67℉ to +257℉)</li>
  <li>Utiliza comunicação OneWire</li>
  <li>Acurácia: ±0.5℃ entre -10℃ to +85℃</li>
  <li>Resolução: 9 ou 12 bits</li>
  </ul>

# Montagem



<p>Disponível em: https://youtu.be/y-rcUpNHFAA</p>



# Testes e resultados

<p align="justify"> 
  O teste foi realizado com uma solução de água, limão e suco em pó. Os seguintes passos foram realizados:
   <ul>
  <li>1) A solução foi aquecida até atingir aproximadamente 60ºC</li>
  <li>2) Misturou-se a solução até ficar com uma aparência homogênea.</li>
  <li>3) Os valores de ph, temperatura e turbidez foram medidos até a solução entrar em equilíbrio térmico com o ambiente.</li>
  </ul>
  </p>
  
  <h3> Resultados </h3>
  
 <p align="justify"> 
  Os valores medidos são apresentados a seguir. É possível observar que quando a temperatura diminui, a medida de pH aumenta. 
    Os valores de turbidez diminuem com o passar do tempo devido ao processo de decantação e apresentam um ruído maior.
</p>



![Resultados](https://github.com/FelipeCamargoXavier/water-quality/blob/main/images/resultados_teste.svg)





<p>Disponível em: https://youtu.be/EplQrjDQ0Z4</p>

  


# Referências

<ul>
  <li>[1] https://wiki.dfrobot.com/Waterproof_DS18B20_Digital_Temperature_Sensor__SKU_DFR0198_</li>
  <li>[2] https://wiki.dfrobot.com/Turbidity_sensor_SKU__SEN0189</li>
   <li>[3] https://wiki.dfrobot.com/Gravity__Analog_pH_Sensor_Meter_Kit_V2_SKU_SEN0161-V2 </li>
</ul>


