#include <Wire.h>
#include <FaBoLCDmini_AQM0802A.h>
FaBoLCDmini_AQM0802A lcd;

const int iLCD_cont = 100;  // LCDのｺﾝﾄﾗｽﾄ設定(0～255) //LCD contrast setting (0 to 255)
const int iDebug_mode = 0;  // 0:通常ﾓｰﾄﾞ 1:ﾃﾞﾊﾞｯｸﾞﾓｰﾄﾞ, 2:波形ﾌﾟﾛｯﾄﾓｰﾄﾞ
const int iDebug_spd =1000; // ﾃﾞﾊﾞｯｸﾞ時のｳｪｲﾄﾀｲﾏｰ[ms]
//const int Lcd_cnt = 6;      // LCDｺﾝﾄﾗｽﾄﾋﾟﾝ   
const int Pulse_Out = 5;    // ﾊﾟｽﾙ出力ﾋﾟﾝ
const int Button_input = A0;// ﾎﾞﾀﾝ付きﾎﾞｰﾄﾞｲﾝﾌﾟｯﾄﾋﾟﾝ
int iButton_No_Res = 0;     // 一回前のﾎﾞﾀﾝNo.
int iButton_No = 0;         // 現在のﾎﾞﾀﾝNo.
int iReturn_Once_flg;       // 戻り値一回ﾌﾗｸﾞ
int iLp_cnt = 0;            // ﾙｰﾌﾟｶｳﾝﾀ
float fDuty = 0.1;          // ﾃﾞﾌｫﾙﾄDuty[%]
float fDuty_Max = 100;      // Duty[%]の最大値
float fDuty_Min = 0;        // Duty[%]の最小値
float fHz = 1;              // ﾃﾞﾌｫﾙﾄ周波数[Hz]
float fHz_Max = 99999;        // 周波数[Hz]
float fHz_Min = 0.01;       // 周波数[Hz]
long lPeriod = 0;           // 周期[usec]
long lOntime = 0;           // on時間[usec]
long lOfftime = 0;          // off時間[usec]
int iOnDelay_ms = 0;        // Ondelay[ms]の抽出
int iOnDelay_us = 0;        // Ondelay[us]の抽出
int iOffDelay_ms = 0;       // Offdelay[ms]の抽出
int iOffDelay_us = 0;       // Offdelay[us]の抽出
int iOn_flg = 0;            // (ｼﾞｪﾈﾚｰﾀ出力) 0:off, 1:出力on 
int iSetting_mode_flg = 1;  // (ｾｯﾃｨﾝｸﾞﾓｰﾄﾞ)0:off, 1:onﾞ
int iSet = 0;               // ｾｯﾄﾎﾞﾀﾝ監視
int iOnce_flg = 0;          // 一回だけ起動ﾌﾗｸﾞ
int iOnOff = 0;             // 0:Off, 1:On
int iMode = 0;              // 0:Pulseﾓｰﾄﾞ, 1:Settingﾓｰﾄﾞ
int iYajirushi = 0;         // 方向ｷｰ押下時(1:左, 2:上, 3:右, 4:下)

//*****************************************************
//*** ﾎﾞﾀﾝ対応表                                    ***          
//***                                               ***
//***    2                                          ***
//***  1   3  5  6  7                               ***
//***    4                                          ***
//***                                               ***
//***  1:左へ移動                                   ***       
//***  2:数値上昇                                   ***      
//***  3:数値下降                                   ***      
//***  4:右へ移動                                   ***       
//***  5:On/Off切り替え                             ***             
//***  6:Settingﾓｰﾄﾞ                                ***          
//***  7:Pulse出力ﾓｰﾄﾞ                              ***               
//***                                               ***
//*****************************************************
//*** int iOnOff = 0;             // 0:Off, 1:On
//*** int iMode = 0;              // 0:Pulseﾓｰﾄﾞ, 1:Settingﾓｰﾄﾞ
//*****************************************************

void setup() {
  // put your setup code here, to run once:

  // ｼﾘｱﾙ通信設定
  Serial.begin(9600);

  // ﾃﾞﾊﾞｯｸﾞﾓｰﾄﾞ1時(ｺﾒﾝﾄ出力用) //Debug mode 1 o'clock (for comment output)
  if (iDebug_mode == 1){
     Serial.print("*** ﾃﾞﾊﾞｯｸﾞﾓｰﾄﾞ1 ***\n");
  }

  // ﾃﾞﾊﾞｯｸﾞﾓｰﾄﾞ2時(波形出力用)
  if (iDebug_mode == 2){
     Serial.print("*** ﾃﾞﾊﾞｯｸﾞﾓｰﾄﾞ2 ***\n");
  }

  // ﾘｷｯﾄﾞｸﾘｽﾀﾙ(LCD)初期ﾏｽ目設定
  lcd.begin();

  // ﾋﾟﾝ入出力設定
//  pinMode(Lcd_cnt, OUTPUT);           // LCDｺﾝﾄﾗｽﾄ設定ﾋﾟﾝ指定
  pinMode(Pulse_Out, OUTPUT);         // ｼﾞｪﾈﾚｰﾀ出力設定
//  analogWrite(Lcd_cnt, iLCD_cont);    // LCDのｺﾝﾄﾗｽﾄ設定(default 100で丁度よさそう)

  // 初期ﾃﾞｰﾀ変換処理
  lPeriod = 1000000 / fHz;                  // 周期 [usec] //period
  lOntime = lPeriod * fDuty / 100 - 4;      // On時間[usec]
  lOfftime = lPeriod - lOntime - 180;       // Off時間[usec]

  iOnDelay_ms = lOntime / 100000;
  iOnDelay_us = lOntime % 100000;

  iOffDelay_ms = lOfftime / 100000;
  iOffDelay_us = lOfftime % 100000;

}

void loop() {
  // put your main code here, to run repeatedly:
  static int iButton;     // ﾎﾞﾀﾝNo.戻り値
  int iRtn;               // 関数戻り値

  //**********************************
  //*****   押下されたﾎﾞﾀﾝ認識   *****
  //**********************************    
  iButton = Controler();

  if (iDebug_mode == 1){
    delay(iDebug_spd);                                      // ﾃﾞﾊﾞｯｸﾞ時はゆっくり実行
    Serial.print("Controler戻り値(押下ﾎﾞﾀﾝ):");      
    Serial.print(iButton);
    Serial.print("\n");
  }

  //*******************************************************
  //*****   押下されたﾎﾞﾀﾝに対する振る舞いの場合分け  *****
  //*******************************************************
  //******************************************
  //*** ﾊﾟﾙｽ : on/off, ﾓｰﾄﾞ: Pulse/Setting ***
  //******************************************
  if ( Button_Act(iButton) == 1){
    Serial.print("Button_Act関数にて予期せぬ異常が発生しました\n");
  }else{
    if (iDebug_mode == 1){
      Serial.print("Button_Act()正常終了\n");   

      // 押下ﾎﾞﾀﾝ
      Serial.print("押下ﾎﾞﾀﾝ  :");
      Serial.print(iButton);
      Serial.print("\n");

      // Pulse on/off
      if (iOnOff == 0 ){
        Serial.print("Pulse   :off\n");
      }else{
        Serial.print("Pulse   :on\n");
      }

      // ﾓｰﾄﾞ Pulse/Setting
      if (iMode == 0 ){
        Serial.print("ﾓｰﾄﾞ    :Pulseﾓｰﾄﾞ\n");
      }else{
        Serial.print("ﾓｰﾄ ﾞ    :Settingﾓｰﾄﾞ\n");
      }
    }
  }

  //**************************
      //*****   画面表示処理 *****
  //**************************    
  // LCD表示
  if (iOnce_flg == 0){
    iOnce_flg = 1;        // 更新は何かあった場合のみ
    Lcd_fg();             // LCD表示
    if (iDebug_mode == 1){
      Serial.print("*** LCD書込み終了 ***\n");
    }
  }

  // ﾊﾟﾙｽ出力関数
  Function_Generator();
  if (iDebug_mode == 1){
    Serial.print("Function Generator関数終了\n");
  }
}

//**************************************************************
//********            Funtion Generator関数        *************
//**************************************************************
//*** ●機能                                                 ***
//*** 指定周波数, Dutyのﾊﾟﾙｽ出力                             ***
//*** ●戻り値                                               ***
//*** 中断終了：1                                            ***
//*** 正常終了：0                                            ***
//**************************************************************

int Function_Generator(){
    
  if (iDebug_mode == 1){
    Serial.print("Ontime:");             
    Serial.print(lOntime);               
    Serial.print("usec\n");              
    Serial.print("Offtime:");            
    Serial.print(lOfftime);              
    Serial.print("usec\n");              
    if (iOnOff == 0){
      Serial.print("ｼﾞｪﾈﾚｰﾀ:Off\n");     
    }else{
      Serial.print("ｼﾞｪﾈﾚｰﾀ:On\n");      
    }
  }

  //****************
  //*** ﾊﾟﾙｽ制御 ***
  //****************

  //***********************
  //***** off時間処理 *****
  //***********************

  // ｼﾞｪﾈﾚｰﾀOffであれば、波形出力ﾅｼ
  if (iOnOff == 0){
    digitalWrite(Pulse_Out, LOW);       // off
    if (iDebug_mode == 2){
      Serial.print("0\n");        // 波形ﾌﾟﾛｯﾄ用
    }
    return;                             // ｼﾞｪﾈﾚｰﾀOffであれば、波形出力ﾅｼ
  }

  if (lOfftime > 0){

    digitalWrite(Pulse_Out, LOW);       // off
    if (iDebug_mode == 2){
      Serial.print("0\n");        // 波形ﾌﾟﾛｯﾄ用
    }
    if (lOfftime >= 1000){

      delay(iOffDelay_ms);                  // off時間[ms]
      delayMicroseconds(iOffDelay_us);      // off時間[us]

    }else{
      delayMicroseconds(iOffDelay_us);       // off時間
    }

    if (iDebug_mode == 2){
      Serial.print("0\n");        // 波形ﾌﾟﾛｯﾄ用
    }
  }

  //**********************
  //***** on時間処理 *****
  //**********************

  if (lOntime > 0){
    //***** 波形On *****
    digitalWrite(Pulse_Out, HIGH);       // 波形On Start

    if (iDebug_mode == 1){
      Serial.print("波形On Start\n");   // 波形ﾌﾟﾛｯﾄ用
    }
    
    if (iDebug_mode == 2){
      Serial.print("5\n");              // 波形ﾌﾟﾛｯﾄ用
    }

    if (lOntime >= 1000){
      // delayの代わりにdelayMicrosecondsを使用
      delay(iOnDelay_ms);                  // on時間[ms]

//      for ( int iLp =0; iLp < iDelay_ms; iLp++ ){
//        delayMicroseconds(1000);      // on時間[us]
//      }
      delayMicroseconds(iOnDelay_us);      // on時間[us]
  
    }else{
      delayMicroseconds(iOnDelay_us);        // on時間
    }

    //***** 波形Off *****
    digitalWrite(Pulse_Out, LOW);        // 波形Off Start

    if (iDebug_mode == 1){
      Serial.print("波形Off Start\n");  // 波形ﾌﾟﾛｯﾄ用
    }
    
    if (iDebug_mode == 2){
      Serial.print("5\n");              // 波形ﾌﾟﾛｯﾄ用
    }
  }
}


//*****************************************************
//********            Lcd_fg関数          *************
//*****************************************************
//*** ●引数                                        ***
//*** ナシ                                          ***
//*** ●機能                                        ***
//*** Freq(周波数), Duty設定値表示                  ***
//*** ●戻り値                                      ***
//*** ナシ                                          ***
//*****************************************************

void Lcd_fg(){
  // 変数宣言
  static int iOnce_Set;

    if (iDebug_mode == 1){
      Serial.print("Lcd_fg()関数内部 iOnce_set:");    
      Serial.print(iOnce_Set);    
      Serial.print("\n");
    }
  //***********************
  //***** LCD画面表示 *****  
  //***********************

  //*** 初期化 ***
  lcd.clear();              // LCD表示ｸﾘｱ
  lcd.begin();          // ﾏｽ目ｾｯﾄ
  lcd.command(0x38);
 lcd.command(0x39);
 lcd.command(0x14);
 lcd.command(0x73);
 lcd.command(0x51);
 lcd.command(0x6c);
 lcd.command(0x38);
 lcd.command(0x01);
 lcd.command(0x0c);
 // Print a message to the LCD.
// lcd.print("hello!");
// delay(5000);

  // 場合分け
  if (iMode == 1){

    //********************
    //*** Setting mode ***
    //********************
    Setting_mode(iOnce_Set);         // ｾｯﾃｨﾝｸﾞﾓｰﾄﾞ関数呼び出し
    iOnce_Set = 1;

  }else{
    //******************
    //*** Pulse mode ***
    //******************

    // 初期化
    iOnce_Set = 0;

    //*****************
    //*** Pulseﾓｰﾄﾞ ***
    //*****************

    //*** LCD書込み周波数(Freq)[Hz] ***
    lcd.clear(); 
    lcd.setCursor(0, 0);      // LCD書込み開始位置ｾｯﾄ
    if (iOnOff == 0){
      lcd.print("OffFq:");     // LCD書込み
    }else{
      lcd.print("On Fq:");     // LCD書込み
    } 

    if (fHz < 1){
      lcd.setCursor(10, 0);    // LCD書込み開始位置ｾｯﾄ
    }else if (fHz < 10){
      lcd.setCursor(10, 0);    // LCD書込み開始位置ｾｯﾄ
    }else if (fHz < 100){
      lcd.setCursor(9, 0);    // LCD書込み開始位置ｾｯﾄ
    }else if (fHz < 1000){  
      lcd.setCursor(8, 0);    // LCD書込み開始位置ｾｯﾄ
    }else if (fHz < 10000){  
      lcd.setCursor(7, 0);    // LCD書込み開始位置ｾｯﾄ
    }else{  
      lcd.setCursor(6, 0);    // LCD書込み開始位置ｾｯﾄ
    }
    
    lcd.print(fHz);           // LCD書込み
    lcd.setCursor(14, 0);     // LCD書込み開始位置ｾｯﾄ
    lcd.print("Hz");          // LCD書込み

    //*** LCD書込みDuty[%] ***
    lcd.setCursor(0, 1);      // LCD書込み開始位置ｾｯﾄ
    lcd.print(" Duty:");     // LCD書込み

    if (fDuty < 1){
      lcd.setCursor(10, 1);    // LCD書込み開始位置ｾｯﾄ
    }else if (fDuty < 10){
      lcd.setCursor(10, 1);    // LCD書込み開始位置ｾｯﾄ
    }else if (fDuty < 100){
      lcd.setCursor(9, 1);    // LCD書込み開始位置ｾｯﾄ
    }else{
      lcd.setCursor(8, 1);    // LCD書込み開始位置ｾｯﾄ
    }

    lcd.print(fDuty);         // LCD書込み
    lcd.setCursor(14, 1);     // LCD書込み開始位置ｾｯﾄ
    lcd.print("%");           // LCD書込み
  }                           // Pulseﾓｰﾄﾞ End
}                             // LCD_fg関数 End

//*****************************************************
//********            Controler関数          **********
//*****************************************************
//*** ●機能                                        ***
//*** 押されているﾎﾞﾀﾝNoを返す                      ***
//*** 連続*回範囲内であればﾎﾞﾀﾝOn判定               ***
//*** それ以降は1015以上でﾎﾞﾀﾝOff判定               ***
//***                                               ***
//***●ﾎﾞﾀﾝ, 番号 対比表                            ***
//***                                               ***
//***    2                                          ***
//***  1   3  5  6  7                               ***
//***    4                                          ***
//***                                               ***
//***●戻り値                                       ***
//*** 型 : int                                      ***
//*** 0:何も押されていない(または電圧異常)          ***
//*** 1～7:ﾎﾞﾀﾝ押下(対比表参照)                     ***
//*****************************************************

int Controler(){

  //ﾛｰｶﾙ変数宣言
  int iInput_Vol = 1023;         // 入力電圧取得
  int iCont_Max = 2;             // ﾎﾞﾀﾝ押下判定Max回数
  int iKyoyo = 5;                // ﾎﾞﾀﾝ押下時の許容誤差 
  int iLoop;                     // ﾙｰﾌﾟｶｳﾝﾀ
  int iReturn_No = 0;            // 仮戻り値
  int iCont_Cnt = 2;             // ﾎﾞﾀﾝ押下時のｶｳﾝﾀ(誤動作防止用)
  static int iOutput_No = 0;     // 戻り値
  const int iVb[8] = {1020,     // Voltage of Button 0 pressing
                      224,      // Voltage of Button 1 pressing
                      193,      // Voltage of Button 2 pressing
                      161,      // Voltage of Button 3 pressing
                      125,      // Voltage of Button 4 pressing
                      86,      // Voltage of Button 5 pressing
                      43,       // Voltage of Button 6 pressing
                      0};       // Voltage of Button 7 pressing


  //
  // 同一ﾎﾞﾀﾝ連続押下判定
  // 複数回連続で押下されたら、
  // 一度だけ戻り値に0以外を返す
  // 

  // 変数初期化
  iCont_Cnt = 0;
  while(iCont_Cnt < 9999){
    // ｱﾅﾛｸﾞ電圧読み取り
    iInput_Vol = analogRead(Button_input);

    if (iDebug_mode == 1){
      Serial.print("ﾎﾞﾀﾝ電圧：");
      Serial.print(iInput_Vol);
      Serial.print("\n");
    }
    
    // ﾎﾞﾀﾝ押下判定
    for ( iLoop = 0; iLoop <= 8; iLoop++){
        if (iVb[iLoop] <= iInput_Vol + iKyoyo && iInput_Vol - iKyoyo <= iVb[iLoop]){
        iButton_No = iLoop;
      }  
    }

    if (iDebug_mode == 1){
      Serial.print("ﾎﾞﾀﾝNo：");
      Serial.print(iButton_No);
      Serial.print("\n");
    }

    if (iButton_No == iOutput_No){
      return 0;
    }
    
    if (iButton_No == 0){
      // 変数初期化
      iOutput_No = 0;
      
      iReturn_No = iButton_No;        // 戻り値ｾｯﾄ
      return iReturn_No;              // 戻り値
    }else{
      if (iButton_No == iButton_No_Res){
        // 一回前と同じﾎﾞﾀﾝが押下された場合
        iCont_Cnt++;
        if (iCont_Cnt >= iCont_Max ){
          // 指定回数以上の場合
          iReturn_No = iButton_No;    // 押下判定値を入力
          iOutput_No = iReturn_No;    // 出力変数として値を保管         
          return iOutput_No;
        }
      }
    }
    iButton_No_Res = iButton_No;    // 一回前の値をｾｯﾄ
  }

  if (iDebug_mode == 1){
    Serial.print("1回ｷﾘﾌﾗｸﾞ:");
    Serial.print(iReturn_Once_flg);
    Serial.print("\n");
  }
}

//******************************************
//***        Button_Act関数              ***                                
//******************************************
//*** ●機能                                       
//*** ﾎﾞﾀﾝが押下された場合の処理                                       
//*** ●引数                                       
//*** int iButton                                        
//*** ●戻り値                                       
//*** 0:正常終了, 1:異常発生                                       
//******************************************
//  iOnOff  // 0:Off, 1:On
//  iMod    // 0:Pulseﾓｰﾄﾞ, 1:Settingﾓｰﾄﾞ
//******************************************

int Button_Act(int iButton){

  switch(iButton){
    //**********************
    //*** 0 ﾎﾞﾀﾝ無操作時 ***
    //**********************
    case 0:
      // 無処理 
      return 0;

    //****************************
    //*** 1【左】 ﾎﾞﾀﾝ無操作時 ***
    //****************************
    case 1:
      if (iMode == 1){
        iYajirushi = 1;

        // 再表示
        iOnce_flg = 0;

      }else{
        // 無処理 
      }
      return 0;

    //**********************************
    //*** 2【数値上昇】 ﾎﾞﾀﾝ無操作時 ***
    //**********************************
    case 2:
      if (iMode == 1){
        iYajirushi = 2;

        // 再表示
        iOnce_flg = 0;
      
      }else{
        // 無処理 
      }
      return 0;

    //****************************
    //*** 3【右】 ﾎﾞﾀﾝ無操作時 ***
    //****************************
    case 3:
      if (iMode == 1){
        iYajirushi = 3;

        // 再表示
        iOnce_flg = 0;
      
      }else{
        // 無処理 
      }
      return 0;

    //**********************************
    //*** 4【数値下降】 ﾎﾞﾀﾝ無操作時 ***
    //**********************************
    case 4:
      if (iMode == 1){
        iYajirushi = 4;

        // 再表示
        iOnce_flg = 0;

      }else{
        // 無処理 
      }
      return 0;

    //*********************************
    //*** 5【On/Off切替】ﾎﾞﾀﾝ押下時 ***
    //*********************************
    case 5:
            
      if (iMode == 0){
 
        // On/Off切り替え
        if (iOnOff == 0){
          iOnOff = 1;
        }else{
          iOnOff = 0;
        }

        // 再表示
        iOnce_flg = 0;
      }else{
        // 無処理
      }
      return 0;
  
    //**********************************
    //*** 6【ｾｯﾃｨﾝｸﾞﾓｰﾄﾞ】ﾎﾞﾀﾝ押下時 ***
    //**********************************
    case 6:
      Serial.print("6番押下\n");

      if (iMode == 0){
        // ﾓｰﾄﾞ切替
        iMode = 1;
        iOnOff = 0;

        // 再表示
        iOnce_flg = 0;
      }else{
        // 無処理
      }
      return 0;

  //********************************
  //*** 7【Pulseﾓｰﾄﾞ】ﾎﾞﾀﾝ押下時 ***
  //********************************
    case 7:

      if (iMode == 0){
        // 無処理
      }else{
        // ﾓｰﾄﾞ切替
        iMode = 0;
        iOnOff = 0;

        // 再表示
        iOnce_flg = 0;
      }
      return 0;
      
    default:
      return 1;
  }
}

//*****************************************************
//********         Setting_mode関数          **********
//*****************************************************
//*** ●引数                                        ***
//*** int iOnce_Set(0:初回呼出, 1:2回目以降呼出)    ***
//*** ●機能                                        ***
//*** Freq(周波数), Duty設定値変更                  ***
//*** ●戻り値                                      ***
//*** ﾅｼ                                            ***
//*** int iOnce_Set(0:初回呼出, 1:2回目以降呼出)    ***
//*****************************************************
int Setting_mode(int iOnce_Setting){

  // 変数宣言
  static int iCursor_Pos; // ｶｰｿﾙ位置
  int iIncDec = 0;        // 0:ｲﾝｸﾘﾒﾝﾄ, 1:ﾃﾞｸﾘﾒﾝﾄ

  //*******************
  //*** ｾｯﾃｨﾝｸﾞﾓｰﾄﾞ ***
  //*******************
  if (iOnce_Setting == 0){
    //*** Setting ﾓｰﾄﾞ 1回目 ***
    
    // 変数初期化
    iCursor_Pos = 0;

    // ｾｯﾃｨﾝｸﾞﾓｰﾄﾞ開始表示
    lcd.clear();
    lcd.setCursor(0, 0);        // LCD書込み開始位置ｾｯﾄ
    lcd.print("Setting mode");  // LCD書込み
    lcd.setCursor(0, 1);        // LCD書込み開始位置ｾｯﾄ
    lcd.print("    Start");     // LCD書込み
    iOnce_Setting = 1;          // 一度きり表示
    delay(2000);
    iOnce_flg = 0;              // LCD再表示       
  }else{
    //*** Setting ﾓｰﾄﾞ 2回目以降 ***

    // ｾｯﾃｨﾝｸﾞﾓｰﾄﾞ本開始
    lcd.print("SetFq:");     // LCD書込み
    if (fHz < 1){
      lcd.setCursor(10, 0);    // LCD書込み開始位置ｾｯﾄ
    }else if (fHz < 10){
      lcd.setCursor(10, 0);    // LCD書込み開始位置ｾｯﾄ
    }else if (fHz < 100){
      lcd.setCursor(9, 0);    // LCD書込み開始位置ｾｯﾄ
    }else if (fHz < 1000){
      lcd.setCursor(8, 0);    // LCD書込み開始位置ｾｯﾄ
    }else if (fHz < 10000){
      lcd.setCursor(7, 0);    // LCD書込み開始位置ｾｯﾄ
    }else{
      lcd.setCursor(6, 0);    // LCD書込み開始位置ｾｯﾄ
    }  
      
    lcd.print(fHz);           // LCD書込み
    lcd.setCursor(14, 0);     // LCD書込み開始位置ｾｯﾄ
    lcd.print("Hz");          // LCD書込み

    lcd.setCursor(0, 1);      // LCD書込み開始位置ｾｯﾄ
    lcd.print(" Duty:");     // LCD書込み

    if (fDuty < 1){
      lcd.setCursor(10, 1);    // LCD書込み開始位置ｾｯﾄ
    }else if (fDuty < 10){
      lcd.setCursor(10, 1);    // LCD書込み開始位置ｾｯﾄ
    }else if (fDuty < 100){
      lcd.setCursor(9, 1);    // LCD書込み開始位置ｾｯﾄ
    }else{
      lcd.setCursor(8, 1);    // LCD書込み開始位置ｾｯﾄ
    }

    lcd.print(fDuty);         // LCD書込み
    lcd.setCursor(14, 1);     // LCD書込み開始位置ｾｯﾄ
    lcd.print("%");           // LCD書込み

    //***************************
    //*** ｾｯﾃｨﾝｸﾞﾓｰﾄﾞ編集処理 ***
    //***************************
    switch (iYajirushi){
      case 1:   // 左ｷｰ押下時
        if (iCursor_Pos == 0){
          iCursor_Pos = 9;  
        }else{
          iCursor_Pos--;
        }

        iOnce_flg = 0;              // LCD再表示       
        break;
        
      case 2:   // 上ｷｰ押下時

        // 処理は後半
        iOnce_flg = 0;              // LCD再表示       
        iIncDec = 0;                // ｲﾝｸﾘﾒﾝﾄｾｯﾄ
        break;
  
      case 3:   // 右ｷｰ押下時
        if (iCursor_Pos == 9){
          iCursor_Pos = 0;  
        }else{
          iCursor_Pos++;
        }

        iOnce_flg = 0;              // LCD再表示       
        break;

      case 4:   // 下ｷｰ押下時

        // 処理は後半
        iOnce_flg = 0;              // LCD再表示       
        iIncDec = 1;                // ﾃﾞｸﾘﾒﾝﾄｾｯﾄ
        break;

    }

    if (iDebug_mode == 1){
      Serial.print("方向ｷｰ:");
      Serial.print(iYajirushi);
      Serial.print("\n");

      Serial.print("ｶｰｿﾙ位置:");
      Serial.print(iCursor_Pos);
      Serial.print("\n");
    }

//--------------------------------------------------------------

    //********************************
    //*** iCursor_Pos別LCD表示位置 ***
    //********************************

    // 1段目ｶｰｿﾙ表示処理
    if (iCursor_Pos <= 2){                    // 0, 1, 2の場合
      lcd.setCursor(iCursor_Pos + 8, 0);      // ｶｰｿﾙ位置ｾｯﾄ
      lcd.cursor();                           // ｶｰｿﾙ書込み
    }else if (iCursor_Pos <= 4){              // 3, 4の場合
      lcd.setCursor(iCursor_Pos + 9, 0);      // ｶｰｿﾙ位置ｾｯﾄ
      lcd.cursor();                           // ｶｰｿﾙ書込み
    }
   
    // 2段目ｶｰｿﾙ表示処理
    if (iCursor_Pos >= 8){                     // 8, 9の場合
      lcd.setCursor(iCursor_Pos + 4, 1);      // ｶｰｿﾙ位置ｾｯﾄ
      lcd.cursor();                           // ｶｰｿﾙ書込み
    }else if (iCursor_Pos >= 5){              // 5, 6, 7の場合
      lcd.setCursor(iCursor_Pos + 3, 1);      // ｶｰｿﾙ位置ｾｯﾄ
      lcd.cursor();                           // ｶｰｿﾙ書込み
    }
    lcd.cursor();                           // ｶｰｿﾙ書込み

//--------------------------------------------------------------

    if (iYajirushi == 2 || iYajirushi == 4 ){
      if (iCursor_Pos <= 4){      
        fHz = fSetting_IO(iCursor_Pos, iIncDec);
      }else{
        fDuty = fSetting_IO(iCursor_Pos, iIncDec);
      }
    }

    delay(100);
  }

  // 変数初期化
  iYajirushi = 0;

  // input 情報からのﾃﾞｰﾀ変換
  lPeriod = 1000000 / fHz;                  // 周期 [usec]
  lOntime = lPeriod * fDuty / 100 - 4;      // On時間[usec]
  lOfftime = lPeriod - lOntime - 180;       // Off時間[usec]

  if (lOntime < 0){
    lOntime = 0;
  }

  if (lOfftime < 0){
    lOfftime = 0;
  }

  iOnDelay_ms = lOntime / 100000;
  iOnDelay_us = lOntime % 100000;

  iOffDelay_ms = lOfftime / 100000;
  iOffDelay_us = lOfftime % 100000;

  return iOnce_Setting; 
}

//*****************************************************
//********         表示数値算出関数           *********
//*****************************************************
//*** ●引数                                        ***
//*** int iCursor_Pos(ｶｰｿﾙ表位置)                   ***
//*** int iUpDown(0:上ｷｰ, 1:下ｷｰ)                   ***
//*** ●機能                                        ***
//*** Freq(周波数), Duty設定値変更                  ***
//*** ●戻り値                                      ***
//*** float型 (Hz, Duty)                            ***
//*****************************************************
float fSetting_IO(int iCursor_Pos, int iUpDown){

  float fReturn = 0;    // 計算用 兼 戻り値格納用
  long lEdit = 0;       // 数値計算用
  
  //*********************************
  //*** iCursor_Pos別上下ﾎﾞﾀﾝ処理 ***
  //*********************************

  // 周波数 or Duty変数に代入
  if ( iCursor_Pos <= 4 ){
    fReturn = fHz;
  }else{
    fReturn = fDuty;
  }

  // ｶｰｿﾙ位置別処理
  switch (iCursor_Pos){
    case 0:
    case 5:
      //*** 100の位 ***

      // 数値ﾋﾟｯｸｱｯﾌﾟ
      lEdit = fReturn / 100;

      // 上下ｷｰ別処理      
      if (iUpDown == 0){        // 上ｷｰ
          // 最大値でなければｲﾝｸﾘﾒﾝﾄ
          fReturn = fReturn + 100;
      }else{                    // 下ｷｰ
          // 最大値でなければﾃﾞｸﾘﾒﾝﾄ
          fReturn = fReturn - 100;
      }
      
      break;

    case 1:
    case 6:
      //*** 10の位 ***

      // 数値ﾋﾟｯｸｱｯﾌﾟ
      lEdit = fReturn / 10;
        lEdit = lEdit % 10;

      // 上下ｷｰ別処理      
      if (iUpDown == 0){        // 上ｷｰ
          fReturn = fReturn + 10;
      }else{                    // 下ｷｰ
          fReturn = fReturn - 10;
      }
      
      break;

    case 2:
    case 7:
      //*** 1の位 ***

      // 数値ﾋﾟｯｸｱｯﾌﾟ
        lEdit = fReturn;
        lEdit = lEdit % 10;
        
      // 上下ｷｰ別処理      
      if (iUpDown == 0){        // 上ｷｰ
          // 最大値でなければｲﾝｸﾘﾒﾝﾄ
          fReturn = fReturn + 1;
      }else{                    // 下ｷｰ
          // 最大値でなければﾃﾞｸﾘﾒﾝﾄ
          fReturn = fReturn - 1;
      }
      
      break;

    case 3:
    case 8:
      //*** 小数点以下1位 ***

      // 数値ﾋﾟｯｸｱｯﾌﾟ
      lEdit = fReturn * 10;
      lEdit = lEdit % 10;

      // 上下ｷｰ別処理      
      if (iUpDown == 0){        // 上ｷｰ
          fReturn = fReturn + 0.1;
      }else{                    // 下ｷｰ
          // 最大値でなければﾃﾞｸﾘﾒﾝﾄ
          fReturn = fReturn - 0.1;
      }
      
      break;

    case 4:
    case 9:
      //*** 小数点以下2位 ***

      // 数値ﾋﾟｯｸｱｯﾌﾟ
      lEdit = fReturn * 100;
      lEdit = lEdit % 10;

      // 上下ｷｰ別処理      
      if (iUpDown == 0){        // 上ｷｰ
          fReturn = fReturn + 0.01;
      }else{                    // 下ｷｰ
          fReturn = fReturn - 0.01;
      }
      break;
  }

  // 周波数, Duty の 最大/最小を外れた場合は境界値を代入
  if ( iCursor_Pos <= 4 ){
    if (fReturn >= fHz_Max){
      fReturn = fHz_Max;
    }

    if (fReturn <= fHz_Min){
      fReturn = fHz_Min;
    }
  }else{
    if (fReturn >= fDuty_Max){
      fReturn = fDuty_Max;
    }

    if (fReturn <= fDuty_Min){
      fReturn = fDuty_Min;
    }
  }
  // 編集後のHz/Duty値を返す
  return fReturn;
}
