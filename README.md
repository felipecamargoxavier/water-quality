# Monitoramento de qualidade de água

<h3>Implementação com ARDUINO UNO:</h3>
<ul>
  <li><a href="https://github.com/FelipeCamargoXavier/water-quality/blob/main/turbity-temperature.ino">Monitoramento de turbidez e temperatura</a></li>
  <li><a href="https://github.com/FelipeCamargoXavier/water-quality/blob/main/turbity-temperature-ph.ino">Monitoramento de turbidez, temperatura e pH</a></li>
</ul>


<h3>Componentes:</h3>
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


# Montagem

<p align="center">
    <img src="https://github.com/FelipeCamargoXavier/water-quality/blob/main/images/montagem.jpeg?raw=true" />
</p>

# Referências

<ul>
  <li>[1] https://wiki.dfrobot.com/Waterproof_DS18B20_Digital_Temperature_Sensor__SKU_DFR0198</li>
  <li>[2] https://wiki.dfrobot.com/Turbidity_sensor_SKU__SEN0189</li>
   <li>[3] https://wiki.dfrobot.com/Gravity__Analog_pH_Sensor_Meter_Kit_V2_SKU_SEN0161-V2 </li>
</ul>

