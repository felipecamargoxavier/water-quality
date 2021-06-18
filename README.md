# Monitoramento de qualidade de água

<h3>Implementação:</h3>
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

# Calibração

<h3> Sensor pH SEN0189 </h3>

<p> 
A biblioteca <a href="https://github.com/DFRobot/DFRobot_PH">DFRobot_PH Library</a>  implementa a medição com compensação de temperatura e a calibração do sensor SEN0189. As referências para calibração são as amostras adquiridas com o kit do sensor. A partir das amostras é obtido dois pontos no gráfico <b>Tensão(V) x pH</b> e traçado uma curva linear que relaciona a tensão com a medida de pH. Os dois pontos de referência são salvos na memória EEPROM do Arduino [3].
</p>

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

