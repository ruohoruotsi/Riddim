

void cfftf(int N, double complex_data[], double wrk[]);  // complex transform             
void cfftb(int N, double complex_data[], double wrk[]);	// its inverse                   
void cffti(int N, double wrk[]);			// initializer of the above routine pair

void rfftf(int N, double data[], double wrk[]);	// real transform                   
void rfftb(int N, double data[], double wrk[]);	// its inverse                      
void rffti(int N, double wrk[]);			// initializer for rfft             

void sint(int N, double data[], double wrk[]);	// sine transform                   
void sinti(int N, double wrk[]);			// initializer for sint             

void cost(int N, double data[], double wrk[]);	// cosine transform                 
void costi(int N, double wrk[]);			// initializer for cost             

void sinqf(int N, double data[], double wrk[]);	// quarter-wave sine transform      
void sinqb(int N, double data[], double wrk[]);	// its inverse                      
void sinqi(int N, double wrk[]);			// initializer for sinq             

void cosqf(int N, double data[], double wrk[]);	// quarter-wave cosine transform    
void cosqb(int N, double data[], double wrk[]);	// its inverse                      
void cosqi(int N, double wrk[]);			// initializer for cosq             

