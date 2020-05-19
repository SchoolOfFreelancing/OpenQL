/**
 * @file   passes.cc
 * @date   04/2020
 * @author Razvan Nane
 * @brief  OpenQL Passes
 * @note   Below passes should eventually be implemented into their own files.
 * @todo-rn Split this file into multiple (pass) files together with folder restructuring
 */

#include "passes.h"
#include "cqasm/cqasm_reader.h"

#include <iostream>

namespace ql
{

    /**
     * @brief   Gets the name of the pass
     * @return  Name of the compiler pass
     */
std::string AbstractPass::getPassName()
{
    return passName;
}

    /**
     * @brief  Sets the name of the pass
     * @param  Name of the compiler pass
     */
void AbstractPass::setPassName(std::string name) 
{ 
    passName = name; 
}

    /**
     * @brief   Sets a pass option
     * @param   optionName String option name
     * @param   optionValue String value of the option
     */
void AbstractPass::setPassOption(std::string optionName, std::string optionValue)
{
    DOUT("In AbstractPass::setPassOption");
    
    passOptions->set(optionName, optionValue);  
}

    /**
     * @brief   Initializes the pass options object
     */
void AbstractPass::createPassOptions()
{
    passOptions = new PassOptions(getPassName());
}

    /**
     * @brief  Reader pass constructor
     * @param  Name of the read pass
     */
ReaderPass::ReaderPass(std::string name) 
{ 
    setPassName(name); 
    createPassOptions();
}

    /**
     * @brief  Apply the pass to the input program
     * @param  Program object to be read
     */
void ReaderPass::runOnProgram(ql::quantum_program *programp)
{
    DOUT("run ReaderPass with name = " << getPassName() << " on program " << programp->name);
    ///@todo-rn: call or import the actual reader pass from the openql file
    
    ///@todo-rn: set pass options with platform file, now hard-coded for testing , which I still have to do!
    /*ql::cqasm_reader* reader = new ql::cqasm_reader(new ql::quantum_platform("testPlatform","hardware_config_cc_light.json"), programp);
    
    reader->file2circuit(cqasm_file_path);*/
}

    /**
     * @brief  Writer pass constructor
     * @param  Name of the read pass
     */
WriterPass::WriterPass(std::string name) 
{ 
    setPassName(name); 
    createPassOptions();
}

    /**
     * @brief  Apply the pass to the input program
     * @param  Program object to be write
     */
void WriterPass::runOnProgram(ql::quantum_program *programp)
{
    DOUT("run WriterPass with name = " << getPassName() << " on program " << programp->name);
    ///@todo-rn: call or import the actual reader pass from the openql file
    
    std::string name = programp->name;
    int qubit_count = 0;
    
   
//    if (ql::options::get("unique_output") == "yes")
//  {
//      int vers;
//      vers = bump_unique_file_version(name);
//      if (vers > 1)
//      {
//          name = ( name + to_string(vers) );
//          DOUT("new program name after bump_unique_file_version: " << name << " based on version: " << vers);
//      }
//  }

    std::stringstream ss;
    ss << ql::options::get("output_dir") << "/" << name << ".qasm";
    
    std::stringstream s;
    s << "version 1.0\n";
    s << "# this file has been automatically generated by the OpenQL compiler please do not modify it manually.\n";
    s << "qubits " << qubit_count << "\n";
    
    for (auto& k : programp->kernels)
        s <<'\n' << k.qasm();
    

    IOUT("writing un-scheduled qasm to '" << ss.str() << "' ...");
    ql::utils::write_file(ss.str(), s.str());
    DOUT("writing done");
    
}

/** @brief supports a unique file called 'get("output_dir")/name.unique'
 * it is a seed to create unique output files (qasm, report, etc.) for the same program (with name 'name')
 * - when the unique file is not there, it is created with the value 0 (which is then the current value)
 *   otherwise, it just reads the current value from that file
 * 
 * since this may be the first time that the output_dir is used, it warns when that doesn't exist
 * @param name String of the file name
 * @return it then increments the current value by 1, stores it in the file and returns this value
 */
int WriterPass::bump_unique_file_version(std::string name)
{
    std::stringstream ss_unique;
    ss_unique << ql::options::get("output_dir") << "/" << name << ".unique";

    std::fstream ufs;
    int vers;

    // retrieve old version number
    ufs.open (ss_unique.str(), std::fstream::in);
    if (!ufs.is_open())
    {
        // no file there, initialize old version number to 0
        ufs.open(ss_unique.str(), std::fstream::out);
        if (!ufs.is_open())
        {
            FATAL("Cannot create: " << ss_unique.str() << ". Probably output directory " << ql::options::get("output_dir") << " does not exist");
        }
        ufs << 0 << std::endl;
        vers = 0;
    }
    else
    {
        // read stored number
        ufs >> vers;
    }
    ufs.close();

    // increment to get new one, store it for later and return
    vers++;
    ufs.open(ss_unique.str(), std::fstream::out);
    ufs << vers << std::endl;
    ufs.close();

    return vers;
}

    /**
     * @brief  Optimizer pass constructor
     * @param  Name of the optimized pass
     */
OptimizerPass::OptimizerPass(std::string name) 
{ 
    setPassName(name); 
    createPassOptions();
}

    /**
     * @brief  Apply the pass to the input program
     * @param  Program object to be read
     */
void OptimizerPass::runOnProgram(ql::quantum_program *programp)
{
    DOUT("run OptimizePass with name = " << getPassName() << " on program " << programp->name);
    
    if( ql::options::get("optimize") == "yes" )
    {
        IOUT("optimizing quantum kernels...");
        for (auto& k : programp->kernels)
            k.optimize();
    }
}
  
    /**
     * @brief  Scheduler pass constructor
     * @param  Name of the scheduler pass
     */
SchedulerPass::SchedulerPass(std::string name)
{ 
    DOUT("In SchedulerPass::SchedulerPass");
    setPassName(name); 
    createPassOptions(); 
}

    /**
     * @brief  Apply the pass to the input program
     * @param  Program object to be read
     */
void SchedulerPass::runOnProgram(ql::quantum_program *programp)
{
    DOUT("run ReadPass with name = " << getPassName() << " on program " << programp->name);
    
    ///@todo-rn: import the scheduler here and disentangle from platform. Needed to make a difference between prescheduler and rcscheduler, whereas the first can be used just for platform-independent simulations
    //programp->schedule(); 
}

    /**
     * @brief  Construct an object to hold the pass options
     * @param  app_name String with the name of the pass options object
     */
PassOptions::PassOptions(std::string app_name="passOpts")
{
    app = new CLI::App(app_name);

    ///@todo-rn: update this list with meaningful pass options
    // default values
    opt_name2opt_val["write_qasm_files"] = "no";
    opt_name2opt_val["read_qasm_files"] = "no";

    // add options with default values and list of possible values
    app->add_set_ignore_case("--write_qasm_files", opt_name2opt_val["write_qasm_files"], {"yes", "no"}, "write (un-)scheduled (with and without resource-constraint) qasm files", true);
    app->add_set_ignore_case("--read_qasm_files", opt_name2opt_val["read_qasm_files"], {"yes", "no"}, "read (un-)scheduled (with and without resource-constraint) qasm files", true);
}

    /**
     * @brief  Show the values set for the pass options.
     */
void PassOptions::print_current_values()
{
    ///@todo-rn: update this list with meaningful pass options
    std::cout << "write_qasm_files: " << opt_name2opt_val["write_qasm_files"] << std::endl
              << "read_qasm_files: " << opt_name2opt_val["read_qasm_files"] << std::endl;
}

    /**
     * @brief  Displays the help menu to list the available options.
     */
void PassOptions::help()
{
    std::cout << app->help() << std::endl;
}

    /**
     * @brief   Sets a pass option
     * @param   opt_name String option name
     * @param   opt_value String value of the option
     */
void PassOptions::set(std::string opt_name, std::string opt_value)
{
    DOUT("In PassOptions: setting option " << opt_name << " to value " << opt_value << std::endl);
    try
    {
       std::vector<std::string> opts = {opt_value, "--"+opt_name};
       app->parse(opts);
    }
    catch (const std::exception &e)
    {
       app->reset();
       EOUT("Un-known option:"<< e.what());
       throw ql::exception("Error parsing options. "+std::string(e.what())+" !",false);
    }
    app->reset();
}

    /**
     * @brief  Queries an option
     * @param opt_name Name of the options
     * @return Value of the option
     */
std::string PassOptions::get(std::string opt_name)
{
    std::string opt_value("UNKNOWN");
    if( opt_name2opt_val.find(opt_name) != opt_name2opt_val.end() )
    {
        opt_value = opt_name2opt_val[opt_name];
    }
    else
    {
        EOUT("Un-known option:"<< opt_name);
    }
    return opt_value;
}


} // ql
