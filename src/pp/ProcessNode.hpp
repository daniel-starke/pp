/**
 * @file ProcessNode.hpp
 * @author Daniel Starke
 * @copyright Copyright 2015-2018 Daniel Starke
 * @date 2015-03-22
 * @version 2016-12-29
 */
#ifndef __PP_PROCESSNODE_HPP__
#define __PP_PROCESSNODE_HPP__


#include <cstdlib>
#include <iosfwd>
#include <string>
#include <vector>
#include <boost/config/warning_disable.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
/* workaround for boost::phoenix::bind and boost::bind namespace collision */
#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/thread/thread.hpp>
#undef BOOST_BIND_NO_PLACEHOLDERS
#include <pcf/exception/General.hpp>
#include "Process.hpp"
#include "Variable.hpp"
#include "Type.hpp"


namespace pp {


/**
 * Structure to hold a process node element (single leaf of the dependency tree).
 * 
 * @see ProcessNode
 */
struct ProcessElement {
	Process process; /**< Associated process instance. */
	StringLiteralVector initialInput; /**< Initial input file list specifications. */
	bool addInitialInput; /**< True if the initial input file list shall be generated from initialInput. */
	PathLiteralPtrVector input; /**< Initial input file list. */
	
	/** Default constructor. */
	explicit ProcessElement():
		addInitialInput(false)
	{}
	
	/**
	 * Constructor.
	 * 
	 * @param[in,out] p - process instance to use
	 */
	explicit ProcessElement(Process & p):
		process(p),
		addInitialInput(false)
	{}
	
	/**
	 * Constructor.
	 *
	 * @param[in,out] p - process instance to use
	 * @param[in] ii - initial input file list specifications
	 */
	explicit ProcessElement(Process & p, const StringLiteralVector & ii):
		process(p),
		initialInput(ii),
		addInitialInput(false)
	{}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	ProcessElement(const ProcessElement & o):
		process(o.process),
		initialInput(o.initialInput),
		addInitialInput(o.addInitialInput),
		input(o.input)
	{}
	
	/**
	 * Returns the output file list from this process. This may also set internal states accordingly.
	 *
	 * @return output file list
	 */
	PathLiteralPtrVector getOutput() {
		return this->process.getOutputs();
	}
};


/**
 * Dependency tree.
 */
struct ProcessNode {
private:
	size_t parallelInQueue; /**< Number of remaining dependency trees within the execution queue. */
	mutable boost::mutex mutex; /**< Mutex object for parallel execution. */
public:
	/** Leaf value type. */
	typedef ProcessElement ValueType;
	/** Parallel nodes type. */
	typedef std::vector<ProcessNode> ParallelType;
	/** Dependency nodes type. */
	typedef std::vector<ProcessNode> DependencyType;
	/** Callback function type for simple traversal. */
	typedef boost::function2<bool, ValueType &, const size_t> SimpleCallbackType;
	/** Callback function type for dependency traversal. */
	typedef boost::function2<bool, ValueType &, PathLiteralPtrVector &> DependencyCallbackType;
	/** Callback function type for traversal for execution. */
	typedef boost::function2<bool, ValueType &, const ExecutionCallback &> ExecuteNodeCallbackType;
	
	boost::optional<ValueType> value; /**< Optional leaf value. */
	ParallelType parallel; /**< Parallel executable dependencies. */
	DependencyType dependency; /**< Direct dependencies which needs to finish beforehand in reverse order. */
	
	/** Default constructor. */
	explicit ProcessNode() {}
	
	/**
	 * Copy constructor.
	 *
	 * @param[in] o - object to copy
	 */
	ProcessNode(const ProcessNode & o):
		parallelInQueue(o.parallelInQueue),
		value(o.value),
		parallel(o.parallel),
		dependency(o.dependency)
	{}
	
	/**
	 * Assignment operator.
	 *
	 * @param[in] o - object to assign
	 * @return reference to this object for chained operations
	 */
	ProcessNode & operator= (const ProcessNode & o) {
		if (this != &o) {
			parallelInQueue = o. parallelInQueue;
			value = o.value;
			parallel = o.parallel;
			dependency = o.dependency;
		}
		return *this;
	}
	
	/**
	 * Checks whether this node is a value node.
	 *
	 * @return true if this node is a value node, else false
	 */
	bool isValue() const {
		return (this->parallel.empty() && this->dependency.empty());
	}
	
	/**
	 * Checks whether this node is a leaf node.
	 *
	 * @return true if this node is a leaf node, else false
	 */
	bool isLeaf() const {
		return this->dependency.empty();
	}
	
	/**
	 * Traverses the dependency tree depth-first, pre-order.
	 *
	 * @param[in] callback - visitor
	 * @param[in] level - current dependency level (not needed to set by top level functions)
	 * @return true on success, false if a visitor returned false or no visitor was given
	 * @see https://en.wikipedia.org/wiki/Tree_traversal
	 */
	bool traverseTopDown(const SimpleCallbackType & callback, const size_t level = 0) {
		if ( ! callback ) return false;
		/* call own */
		if ( this->value ) {
			if ( ! callback(*(this->value), level) ) return false;
		}
		/* call parallel */
		BOOST_FOREACH(ProcessNode & node, this->parallel) {
			if ( ! node.traverseTopDown(callback, level + 1) ) return false;
		}
		/* call dependency */
		BOOST_REVERSE_FOREACH(ProcessNode & node, this->dependency) {
			if ( ! node.traverseTopDown(callback, level + 1) ) return false;
		}
		return true;
	}
	
	/**
	 * Traverses the dependency tree depth-first, post-order.
	 *
	 * @param[in] callback - visitor
	 * @param[in] level - current dependency level (not needed to set by top level functions)
	 * @return true on success, false if a visitor returned false or no visitor was given
	 * @see https://en.wikipedia.org/wiki/Tree_traversal
	 */
	bool traverseBottomUp(const SimpleCallbackType & callback, const size_t level = 0) {
		if ( ! callback ) return false;
		/* call dependency */
		BOOST_REVERSE_FOREACH(ProcessNode & node, this->dependency) {
			if ( ! node.traverseBottomUp(callback, level + 1) ) return false;
		}
		/* call parallel */
		BOOST_FOREACH(ProcessNode & node, this->parallel) {
			if ( ! node.traverseBottomUp(callback, level + 1) ) return false;
		}
		/* call own */
		if ( this->value ) {
			if ( ! callback(*(this->value), level) ) return false;
		}
		return true;
	}
	
	/**
	 * Traverses the dependency tree in dependency order (similar to depth-first, post-order) while
	 * passing the dependencies to each dependent node.
	 *
	 * @param[in] callback - visitor
	 * @param[in] output - variable to fill with the dependencies for the current node
	 * @param[in] level - current dependency level (not needed to set by top level functions)
	 * @return true on success, false if a visitor returned false, a function failed or no visitor was given
	 */
	bool traverseDependencies(const DependencyCallbackType & callback, PathLiteralPtrVector & output, const size_t level = 0) {
		if ( ! callback ) return false;
		PathLiteralPtrVector passingDeps;
		/* call dependency */
		if ( ! this->isLeaf() ) {
			BOOST_REVERSE_FOREACH(ProcessNode & node, this->dependency) {
				/* output of previous dependency is input for next dependency or will be processed afterwards if no further dependency exists */
				if ( ! passingDeps.empty() ) node.setDependencyInput(passingDeps);
				passingDeps.clear();
				if ( ! node.traverseDependencies(callback, passingDeps, level + 1) ) return false;
			}
		}
		/* call parallel */
		if ( ! this->parallel.empty() ) {
			/* set dependency output to input of parallel process leafs */
			BOOST_FOREACH(ProcessNode & node, this->parallel) {
				node.setDependencyInput(passingDeps);
			}
			/* solve dependencies for parallel nodes */
			if ( this->value ) {
				BOOST_FOREACH(ProcessNode & node, this->parallel) {
					if ( ! node.traverseDependencies(callback, this->value->input, level + 1) ) return false;
				}
			} else {
				BOOST_FOREACH(ProcessNode & node, this->parallel) {
					if ( ! node.traverseDependencies(callback, output, level + 1) ) return false;
				}
			}
		} else if ( this->value ) {
			if (this->value->initialInput.empty() || this->value->addInitialInput) {
				/* add dependency output to own input */
				boost::push_back(this->value->input, passingDeps);
			}
		}
		/* call own */
		if ( this->value ) {
			if ( ! callback(*(this->value), output) ) return false;
		}
		return true;
	}
	
	/**
	 * Traverses the dependency tree in dependency order while passing on after all parallel nodes
	 * finished execution with the given callback.
	 *
	 * @param[in] callback - visitor
	 * @param[in] callNext - function to execute next (called recursively within this function)
	 * @return true on success, false if a visitor returned false, a function failed or no visitor was given
	 */
	bool executeChain(const ExecuteNodeCallbackType & callback, const ExecutionCallback & callNext) {
		boost::mutex::scoped_lock lock(this->mutex);
		this->parallelInQueue = 0;
		lock.unlock();
		/* execute dependency */
		if ( ! this->isLeaf() ) {
			if ( ! this->executeDependency(0, callback, callNext) ) return false;
		} else if ( ! this->parallel.empty() ) {
			/* execute parallel nodes */
			if ( ! this->executeParallel(callback, callNext) ) return false;
		} else if ( this->value ) {
			/* execute self */
			switch (this->value->process.getState()) {
			case Process::IDLE:
				if ( callback ) {
					return callback(*(this->value), callNext);
				}
				break;
			case Process::RUNNING:
				return true;
				break;
			case Process::FINISHED:
			case Process::FAILED:
				break;
			}
		} else {
			BOOST_THROW_EXCEPTION(
				pcf::exception::InvalidValue()
				<< pcf::exception::tag::Message(std::string("Internal dependency tree is broken."))
			);
			return false;
		}
		return true;
	}
private:
	/**
	 * Sets the dependency files for this node from the given input.
	 *
	 * @param[in] input - set dependencies from this list
	 * @see traverseDependencies()
	 */
	void setDependencyInput(const PathLiteralPtrVector & input) {
		if ( this->isLeaf() ) {
			if ( this->isValue() ) {
				/* set own node */
				if ( this->value ) {
					if (this->value->initialInput.empty() || this->value->addInitialInput) {
						boost::push_back(this->value->input, input);
					} else {
						/* already set in Execution::solveDependencies() */
					}
				} else {
					BOOST_THROW_EXCEPTION(
						pcf::exception::InvalidValue()
						<< pcf::exception::tag::Message(std::string("Internal dependency tree is broken."))
					);
				}
			} else {
				/* set parallel nodes */
				BOOST_FOREACH(ProcessNode & node, this->parallel) {
					node.setDependencyInput(input);
				}
			}
		} else {
			/* set dependent node */
			BOOST_REVERSE_FOREACH(ProcessNode & node, this->dependency) {
				node.setDependencyInput(input);
			}
		}
	}
	
	/**
	 * Executes all dependent nodes in sequence or this node's dependent parallel nodes.
	 *
	 * @param[in] index - execute dependency with this index
	 * @param[in] callback - function to call for execution
	 * @param[in] callNext - function to call if all nodes finished execution
	 * @return true on success, else false
	 */
	bool executeDependency(const size_t index, const ExecuteNodeCallbackType & callback, const ExecutionCallback & callNext) {
		const size_t depCount = this->dependency.size();
		if (index < depCount) {
			/* execute dependencies in reverse order */
			if ( ! this->dependency[depCount - index - 1].executeChain(callback, boost::phoenix::bind(&ProcessNode::executeDependency, this, index + 1, callback, callNext)) ) return false;
		} else {
			if ( ! this->executeParallel(callback, callNext) ) return false;
		}
		return true;
	}
	
	/**
	 * Executes all dependent parallel nodes in parallel or this node if no parallel nodes are
	 * defined.
	 *
	 * @param[in] callback - function to call for execution
	 * @param[in] callNext - function to call if all nodes finished execution
	 * @return true on success, else false
	 */
	bool executeParallel(const ExecuteNodeCallbackType & callback, const ExecutionCallback & callNext) {
		if ( ! this->parallel.empty() ) {
			/* execute parallel nodes */
			boost::mutex::scoped_lock lock(this->mutex);
			/* we need to ensure that executeSelf() doesn't exists before we are done queuing all parallel processes */
			this->parallelInQueue = this->parallel.size();
			BOOST_FOREACH(ProcessNode & node, this->parallel) {
				lock.unlock();
				if ( ! node.executeChain(callback, boost::phoenix::bind(&ProcessNode::executeSelf, this, callback, callNext)) ) {
					lock.lock();
					this->parallelInQueue--;
					return false;
				}
				lock.lock();
			}
		} else {
			return this->executeSelf(callback, callNext);
		}
		return true;
	}
	
	/**
	 * Executes this node.
	 * 
	 * @param[in] callback - function to call for execution
	 * @param[in] callNext - function to call if all nodes finished execution
	 * @return true on success, else false
	 */
	bool executeSelf(const ExecuteNodeCallbackType & callback, const ExecutionCallback & callNext) {
		boost::mutex::scoped_lock lock(this->mutex);
		if (this->parallelInQueue > 0) this->parallelInQueue--;
		if (this->parallelInQueue <= 0) {
			if ( this->value ) {
				/* execute self */
				switch (this->value->process.getState()) {
				case Process::IDLE:
					lock.unlock();
					if ( callback ) return callback(*(this->value), callNext);
					break;
				case Process::RUNNING:
					return true;
					break;
				case Process::FINISHED:
				case Process::FAILED:
					lock.unlock();
					if ( callNext ) callNext();
					break;
				}
			} else {
				if (this->dependency.empty() && this->parallel.empty()) {
					BOOST_THROW_EXCEPTION(
						pcf::exception::InvalidValue()
						<< pcf::exception::tag::Message(std::string("Internal dependency tree is broken."))
					);
					return false;
				}
				lock.unlock();
				if ( callNext ) callNext();
			}
		}
		return true;
	}
};


} /* namespace pp */


#endif /* __PP_PROCESSNODE_HPP__ */
