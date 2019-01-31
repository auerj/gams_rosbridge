#include <unistd.h>

#include "madara/knowledge/KnowledgeBase.h"
#include "madara/threads/Threader.h"
#include "gams/controllers/BaseController.h"
#include "gams/loggers/GlobalLogger.h"

// ROS includes
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wreorder"
#endif

#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include "sensor_msgs/Imu.h"
#include "tf2_ros/transform_broadcaster.h"


#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

const double TEST_epsilon = 0.0001;

double round_nearest(double in)
{
  return floor(in + 0.5);
}

#define LOG(expr) \
  std::cout << #expr << " == " << (expr) << std::endl

#define TEST(expr, expect) \
  do {\
    double bv = (expr); \
    double v = round_nearest((bv) * 1024)/1024; \
    double e = round_nearest((expect) * 1024)/1024; \
    bool ok = \
      e >= 0 ? (v >= e * (1 - TEST_epsilon) && v <= e * (1 + TEST_epsilon)) \
             : (v >= e * (1 + TEST_epsilon) && v <= e * (1 - TEST_epsilon)); \
    if(ok) \
    { \
      std::cout << #expr << " ?= " << e << "  SUCCESS! got " << bv << std::endl; \
    } \
    else \
    { \
      std::cout << #expr << " ?= " << e << "  FAIL! got " << bv << " instead" << std::endl; \
    } \
  } while(0)

// DO NOT DELETE THIS SECTION

// begin algorithm includes
// end algorithm includes

// begin platform includes
// end platform includes

// begin thread includes
// end thread includes

// begin transport includes
#include "gams/transports/ros/RosBridge.h"
// end transport includes

// begin filter includes
// end filter includes

// END DO NOT DELETE THIS SECTION

const std::string default_broadcast ("192.168.1.255:15000");
// default transport settings
std::string host ("");
const std::string default_multicast ("239.255.0.1:4150");
madara::transport::QoSTransportSettings settings;

// create shortcuts to MADARA classes and namespaces
namespace controllers = gams::controllers;
typedef madara::knowledge::KnowledgeRecord   Record;
typedef Record::Integer Integer;

const std::string KNOWLEDGE_BASE_PLATFORM_KEY (".platform");
bool plat_set = false;
std::string platform ("debug");
std::string algorithm ("debug");
std::vector <std::string> accents;

// madara commands from a file
std::string madara_commands = "";

// for setting debug levels through command line
int madara_debug_level (-1);
int gams_debug_level (-1);

// number of agents in the swarm
Integer num_agents (-1);

// file path to save received files to
std::string file_path;

// filename to save transport settings to
std::string save_transport;
std::string save_transport_prefix;
std::string save_transport_text;
std::string load_transport_prefix;

// controller settings for controller configuration
gams::controllers::ControllerSettings controller_settings;

void print_usage (char * prog_name)
{
  madara_logger_ptr_log (gams::loggers::global_logger.get (),
    gams::loggers::LOG_ALWAYS,
"\nProgram summary for %s:\n\n" 
"     Loop controller setup for gams\n" 
" [-A |--algorithm type]        algorithm to start with\n" 
" [-a |--accent type]           accent algorithm to start with\n" 
" [-b |--broadcast ip:port]     the broadcast ip to send and listen to\n" 
" [--checkpoint-on-loop]        save checkpoint after each control loop\n" 
" [--checkpoint-on-send]        save checkpoint before send of updates\n" 
" [-c |--checkpoint prefix]     the filename prefix for checkpointing\n" 
" [-d |--domain domain]         the knowledge domain to send and listen to\n" 
" [-e |--rebroadcasts num]      number of hops for rebroadcasting messages\n" 
" [-f |--logfile file]          log to a file\n" 
" [-i |--id id]                 the id of this agent (should be non-negative)\n" 
" [-lt|--load-transport file] a file to load transport settings from\n" 
" [-ltp|--load-transport-prefix prfx] prefix of saved settings\n" 
" [-ltt|--load-transport-text file] a text file to load transport settings from\n" 
" [--madara-level level]        the MADARA logger level (0+, higher is higher detail)\n" 
" [--gams-level level]          the GAMS logger level (0+, higher is higher detail)\n" 
" [--loop-hertz hz]             hertz to run the MAPE loop\n"
" [-L |--loop-time time]        time to execute loop\n"
" [-m |--multicast ip:port]     the multicast ip to send and listen to\n" 
" [-M |--madara-file <file>]    file containing madara commands to execute\n" 
"                               multiple space-delimited files can be used\n" 
" [-n |--num_agents <number>]   the number of agents in the swarm\n" 
" [-o |--host hostname]         the hostname of this process (def:localhost)\n" 
" [-p |--platform type]         platform for loop (vrep, dronerk)\n" 
" [-P |--period period]         time, in seconds, between control loop executions\n" 
" [-q |--queue-length length]   length of transport queue in bytes\n" 
" [-r |--reduced]               use the reduced message header\n" 
" [-s |--send-hertz hertz]      send hertz rate for modifications\n" 
" [-st|--save-transport file] a file to save transport settings to\n" 
" [-stp|--save-transport-prefix prfx] prefix to save settings at\n" 
" [-stt|--save-transport-text file] a text file to save transport settings to\n" 
" [-t |--target path]           file system location to save received files (NYI)\n" 
" [-u |--udp ip:port]           a udp ip to send to (first is self to bind to)\n" 
" [--zmq proto:ip:port]         specifies a 0MQ transport endpoint\n"
"\n",
        prog_name);
  exit (0);
}

// handle command line arguments
void handle_arguments (int argc, char ** argv)
{
  for (int i = 1; i < argc; ++i)
  {
    std::string arg1 (argv[i]);

    if (arg1 == "-A" || arg1 == "--algorithm")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
        algorithm = argv[i + 1];
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-a" || arg1 == "--accent")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
        accents.push_back (argv[i + 1]);
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-b" || arg1 == "--broadcast")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        settings.hosts.push_back (argv[i + 1]);
        settings.type = madara::transport::BROADCAST;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-c" || arg1 == "--checkpoint")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        controller_settings.checkpoint_prefix = argv[i + 1];
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "--checkpoint-on-loop")
    {
      controller_settings.checkpoint_strategy =
        gams::controllers::CHECKPOINT_EVERY_LOOP;
    }
    else if (arg1 == "--checkpoint-on-send")
    {
      controller_settings.checkpoint_strategy =
        gams::controllers::CHECKPOINT_EVERY_SEND;
    }
    else if (arg1 == "-d" || arg1 == "--domain")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
        settings.write_domain = argv[i + 1];
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-e" || arg1 == "--rebroadcasts")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        int hops;
        std::stringstream buffer (argv[i + 1]);
        buffer >> hops;

        settings.set_rebroadcast_ttl (hops);
        settings.enable_participant_ttl (hops);
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-f" || arg1 == "--logfile")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        madara::logger::global_logger->add_file (argv[i + 1]);
        gams::loggers::global_logger->add_file (argv[i + 1]);
      }
      else
        print_usage (argv[0]);

      ++i;
    }

    else if (arg1 == "-i" || arg1 == "--id")
    {
      if (i + 1 < argc && argv[i +1][0] != '-')
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> settings.id;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-lt" || arg1 == "--load-transport")
    {
      if (i + 1 < argc)
      {
        if (load_transport_prefix == "")
          settings.load (argv[i + 1]);
        else
          settings.load (argv[i + 1], load_transport_prefix);
      }

      ++i;
    }
    else if (arg1 == "-ltp" || arg1 == "--load-transport-prefix")
    {
      if (i + 1 < argc)
      {
        load_transport_prefix = argv[i + 1];
      }

      ++i;
    }
    else if (arg1 == "-ltt" || arg1 == "--load-transport-text")
    {
      if (i + 1 < argc)
      {
        if (load_transport_prefix == "")
          settings.load_text (argv[i + 1]);
        else
          settings.load_text (argv[i + 1], load_transport_prefix);
      }

      ++i;
    }
    else if (arg1 == "--madara-level")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> madara_debug_level;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "--gams-level")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> gams_debug_level;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-L" || arg1 == "--loop-time")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> controller_settings.run_time;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "--loop-hertz")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> controller_settings.loop_hertz;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-m" || arg1 == "--multicast")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        settings.hosts.push_back (argv[i + 1]);
        settings.type = madara::transport::MULTICAST;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-M" || arg1 == "--madara-file")
    {
      bool files = false;
      ++i;
      for (;i < argc && argv[i][0] != '-'; ++i)
      {
        std::string filename = argv[i];
        if (madara::utility::file_exists (filename))
        {
          madara_commands += madara::utility::file_to_string (filename);
          madara_commands += ";\n";
          files = true;
        }
      }
      --i;

      if (!files)
        print_usage (argv[0]);
    }
    else if (arg1 == "-n" || arg1 == "--num_agents")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> num_agents;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-o" || arg1 == "--host")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
        host = argv[i + 1];
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-p" || arg1 == "--platform")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        platform = argv[i + 1];
        plat_set = true;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-P" || arg1 == "--period")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> controller_settings.loop_hertz;

        controller_settings.loop_hertz = 1 / controller_settings.loop_hertz;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-q" || arg1 == "--queue-length")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        std::stringstream buffer (argv[i + 1]);
        buffer >> settings.queue_length;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-r" || arg1 == "--reduced")
    {
      settings.send_reduced_message_header = true;
    }
    else if (arg1 == "-t" || arg1 == "--target")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
        file_path = argv[i + 1];
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "-st" || arg1 == "--save-transport")
    {
      if (i + 1 < argc)
      {
        save_transport = argv[i + 1];
      }

      ++i;
    }
    else if (arg1 == "-stp" || arg1 == "--save-transport-prefix")
    {
      if (i + 1 < argc)
      {
        save_transport_prefix = argv[i + 1];
      }

      ++i;
    }
    else if (arg1 == "-stt" || arg1 == "--save-transport-text")
    {
      if (i + 1 < argc)
      {
        save_transport_text = argv[i + 1];
      }

      ++i;
    }
    else if (arg1 == "-u" || arg1 == "--udp")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        settings.hosts.push_back (argv[i + 1]);
        settings.type = madara::transport::UDP;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else if (arg1 == "--zmq")
    {
      if (i + 1 < argc && argv[i + 1][0] != '-')
      {
        settings.hosts.push_back (argv[i + 1]);
        settings.type = madara::transport::ZMQ;
      }
      else
        print_usage (argv[0]);

      ++i;
    }
    else
    {
      print_usage (argv[0]);
    }
  }
}






// perform main logic of program
int main (int argc, char ** argv)
{
  settings.type = madara::transport::MULTICAST;
 
  // handle all user arguments
  handle_arguments (argc, argv);

  if (settings.hosts.size () == 0)
  {
    // setup default transport as multicast
    settings.hosts.resize (1);
    settings.hosts[0] = default_multicast;
  }
  
  // set this once to allow for debugging knowledge base creation
  if (madara_debug_level >= 0)
  {
    madara::logger::global_logger->set_level (madara_debug_level);
  }

  // save transport always happens after all possible transport chagnes
  if (save_transport != "")
  {
    if (save_transport_prefix == "")
      settings.save (save_transport);
    else
      settings.save (save_transport, save_transport_prefix);
  }

  // save transport always happens after all possible transport chagnes
  if (save_transport_text != "")
  {
    if (save_transport_prefix == "")
      settings.save_text (save_transport_text);
    else
      settings.save_text (save_transport_text, save_transport_prefix);
  }

  // create knowledge base and a control loop
  madara::knowledge::KnowledgeBase knowledge;
  
  // begin on receive filters
  // end on receive filters
  
  // begin on send filters
  // end on send filters
  
  // if you only want to use custom transports, delete following
  knowledge.attach_transport (host, settings);
  
  // begin transport creation 
  // add RosBridge factory


  std::vector<std::string> selected_topics {"my_topic"};
  std::map<std::string,std::string> topic_map = {{"my_topic", "sensors.odom"}};
  std::map<std::string,std::string> pub_types = {{"odom", "nav_msgs/Odometry"}};

  settings.read_threads = 1;
  gams::transports::RosBridge * ros_bridge = new gams::transports::RosBridge (
    knowledge.get_id (), settings, knowledge, selected_topics,
    topic_map, pub_types);
  knowledge.attach_transport (ros_bridge);
  // end transport creation
  
  // set this once to allow for debugging controller creation
  if (gams_debug_level >= 0)
  {
    gams::loggers::global_logger->set_level (gams_debug_level);
  }

  controllers::BaseController controller (knowledge, controller_settings);
  madara::threads::Threader threader (knowledge);

  // initialize variables and function stubs
  controller.init_vars (settings.id, num_agents);
  
  std::vector <std::string> aliases;
  
  // begin adding custom algorithm factories
  // end adding custom algorithm factories

  // begin adding custom platform factories
  // end adding custom platform factories
  
  // read madara initialization
  if (madara_commands != "")
  {
    knowledge.evaluate (madara_commands,
      madara::knowledge::EvalSettings(false, true));
  }
  
  // set debug levels if they have been set through command line
  if (madara_debug_level >= 0)
  {
    std::stringstream temp_buffer;
    temp_buffer << "agent." << settings.id << ".madara_debug_level = ";
    temp_buffer << madara_debug_level;

    madara::logger::global_logger->set_level (madara_debug_level);

    // modify the debug level being used but don't send out to others
    knowledge.evaluate (temp_buffer.str (),
      madara::knowledge::EvalSettings (true, true));
  }

  if (gams_debug_level >= 0)
  {
    std::stringstream temp_buffer;
    temp_buffer << "agent." << settings.id << ".gams_debug_level = ";
    temp_buffer << gams_debug_level;

    gams::loggers::global_logger->set_level (gams_debug_level);

    // modify the debug level being used but don't send out to others
    knowledge.evaluate (temp_buffer.str (),
      madara::knowledge::EvalSettings (true, true));
  }

  // initialize the platform and algorithm
  // default to platform in knowledge base if platform not set in command line
  if (!plat_set && knowledge.exists (KNOWLEDGE_BASE_PLATFORM_KEY))
    platform = knowledge.get (KNOWLEDGE_BASE_PLATFORM_KEY).to_string ();
  controller.init_platform (platform);
  controller.init_algorithm (algorithm);

  // add any accents
  for (unsigned int i = 0; i < accents.size (); ++i)
  {
    controller.init_accent (accents[i]);
  }

  /**
   * WARNING: the following section will be regenerated whenever new threads
   * are added via this tool. So, you can adjust hertz rates and change how
   * the thread is initialized, but the entire section will be regenerated
   * with all threads in the threads directory, whenever you use the new
   * thread option with the gpc.pl script.
   **/

  // begin thread creation
  // end thread creation
  
  /**
   * END WARNING
   **/
  
  // run a mape loop for algorithm and platform control
  //controller.run ();

  // wait until the bridge is set up correctly
  ros::init(argc, argv, "rosbridge_node");
  ros::NodeHandle n;


  //1Hz ROS spinning
  ros::Rate r(1);
  while (ros::ok())
  {
    knowledge.print();
    r.sleep();
  }

  // terminate all threads after the controller
  threader.terminate ();
  
  // wait for all threads
  threader.wait ();
  
  // print all knowledge values
  knowledge.print ();

  return 0;
}
