using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using Dapper;
using System.Data.SqlClient;
using System;

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
                    ret = await conn.QuerySingleAsync<ResponseModel>("ResponseGet");
                }
            }
            catch
            {
                ret = new ResponseModel { StandByMillis = 5000, DanceId = -1 };
            }
            return ret;
        }
    }
}
