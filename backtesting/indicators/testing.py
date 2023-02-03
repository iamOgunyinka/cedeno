def indicator_function_in_cpp(config:str):
    pass

class indics():
    def __call__(config:str):
        pass
    def config(config:str):
        pass
class confg():
    indicators_handler = indics()
    
    

if __name__ == "__main__":
    config_handler = confg() 
    
    config_handler.indicators_handler(  [                                                   #default    =   [indicator]
                                            ["BWFS", "static", "5", "80"],                  #BWFS       =   [indicator, mode, time, threshold]
                                            ["EMA"],                                        #EMA        =   [indicator, config_1, config_2, config_3]
                                            ["MACD", "config_1", "config_4", "config_3"]    #MACD       =   [indicator, config_1, config_2, config_3]
                                        ]
                                    )
    
    

