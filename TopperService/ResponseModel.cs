using System.Collections.Generic;

namespace TopperService
{
    public class ResponseModel
    {
        public long StandByMillis { get; set; }

        public int Track { get; set; }
        
        public IEnumerable<long> Steps {get;set;}

    }
}
