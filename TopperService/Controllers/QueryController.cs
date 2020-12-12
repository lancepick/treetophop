using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using Dapper;
using System.Data.SqlClient;
using System.Linq;
using System.Collections.Generic;

namespace TopperService.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class QueryController : ControllerBase
    {
        private IConfiguration _config;
        public QueryController(IConfiguration config)
        {
            _config = config;
        }

        [HttpGet]
        public async Task<ResponseModel> Get()
        {
            ResponseModel ret;
            try
            {
                using (var conn = new SqlConnection(_config.GetConnectionString("default")))
                {
                    var q = await conn.QuerySingleAsync<QueryModel>("ResponseGet");
                    ret = new ResponseModel {
                        StandByMillis = q.StandByMillis,
                        Track = q.Track,
                        Steps = q.Steps.Split(',').Select(s=>long.Parse(s))
                    };
                }
            }
            catch
            {
                ret = new ResponseModel { StandByMillis = 5000, Track = -1, Steps = new List<long>() };
            }
            return ret;
        }
    }
}
